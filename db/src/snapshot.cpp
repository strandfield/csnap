// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "snapshot.h"

#include "sql.h"
#include "sqlqueries.h"
#include "symbolloader.h"
#include "transaction.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

namespace csnap
{

struct PendingData
{
  /**
   * \brief the properties that have yet to be written into the database 
   */
  std::map<std::string, std::string> properties;

  /**
   * \brief ids of files that have yet to be written into the database
   */
  std::vector<FileId> files;

  /**
   * \brief ids of translation units that have yet to be written into the database
   */
  std::vector<TranslationUnit*> translation_units;

  /**
   * \brief include directives that have yet to be written into the database
   */
  std::map<TranslationUnit*, std::vector<Include>> includes;

  /**
   * \brief pointers to symbols that have yet to be written into the database
   */
  std::vector<std::shared_ptr<Symbol>> symbols;

  /**
   * \brief the list of base classes that have yet to be written into the database
   */
  std::map<SymbolId, std::vector<BaseClass>> bases;

  std::vector<SymbolReference> symbol_references;
};

Snapshot::Snapshot(Snapshot&&) = default;

Snapshot::~Snapshot()
{
  if (hasPendingData())
    writePendingData();
}

Snapshot::Snapshot(Database db) : 
  m_database(std::make_unique<Database>(std::move(db)))
{
  if (!m_database->good())
    throw std::runtime_error("snapshot constructor expects a good() database");

  {
    std::vector<File> all_files = select_file(*m_database);

    for (File& f : all_files)
    {
      m_files.add(std::make_unique<File>(std::move(f)));
    }
  }

  {
    std::vector<TranslationUnit> all_units = select_translationunit(*m_database);

    for (TranslationUnit& tu : all_units)
    {
      m_translationunits.add(std::make_unique<TranslationUnit>(std::move(tu)));
    }
  }

}

Database& Snapshot::database() const
{
  return *m_database;
}

Snapshot Snapshot::open(const std::filesystem::path& p)
{
  Database db;
  db.open(p);
  return Snapshot(std::move(db));
}

Snapshot Snapshot::create(const std::filesystem::path& p)
{
  Database db;
  db.create(p);
  
  if (!sql::exec(db, db_init_statements()))
    throw std::runtime_error("failed to create snapshot database");

  return Snapshot(std::move(db));
}

void Snapshot::setProperty(const std::string& key, const std::string& value)
{
  pendingData().properties[key] = value;
}

std::string Snapshot::property(const std::string& key) const
{
  if (hasPendingData())
  {
    auto it = m_pending_data->properties.find(key);
    if (it != m_pending_data->properties.end())
      return it->second;
  }

  return select_info(*m_database, key);
}

/**
 * \brief returns a path with all backward slashes replaced by forward slashes
 */
std::string Snapshot::getCanonicalPath(std::string path)
{
  std::for_each(path.begin(), path.end(), [](char& c) {
    if (c == '\\') c = '/';
    });

  return path;
}

/**
 * \brief reads a file completely into memory
 */
std::string Snapshot::readFile(const std::filesystem::path& filepath)
{
  std::ifstream file{ filepath.string(), std::ios::binary };
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string bytes{ buffer.str() };
  return bytes;
}

File* Snapshot::addFile(File f)
{
  File* file = m_files.add(getCanonicalPath(std::move(f.path)));

  pendingData().files.push_back(FileId(file->id));

  return file;
}

void Snapshot::addFile(std::unique_ptr<File> f)
{
  f->path = getCanonicalPath(std::move(f->path));
  FileId id = m_files.add(std::move(f))->id;
  pendingData().files.push_back(id);
}

void Snapshot::addFiles(const std::vector<File>& files)
{
  for (File f : files)
  {
    addFile(std::move(f));
  }
}

File* Snapshot::getFile(FileId id) const
{
  return files().get(id);
}

File* Snapshot::findFile(const std::string& path) const
{
  bool looks_canonical = std::find(path.begin(), path.end(), '\\') == path.end();

  if (looks_canonical)
    return files().find(path);
  else
    return files().find(getCanonicalPath(path));
}

const FileList& Snapshot::files() const
{
  return m_files;
}

/**
 * \brief saves a copy of every file in the database
 */
void Snapshot::addFilesContent()
{
  insert_file_content(*m_database, files().all());
}

std::shared_ptr<FileContent> Snapshot::getFileContent(FileId f)
{
  std::shared_ptr<FileContent> result = m_filecontent_cache.find(f);

  if (result)
    return result;

  File* file = files().get(f);

  if (!file)
    return nullptr;

  std::string content = select_content_from_file(*m_database, f);

  if (content.empty())
    return nullptr;

  result = std::make_shared<FileContent>(file, std::move(content));

  m_filecontent_cache.insert(result);

  return result;
}

void Snapshot::addTranslationUnits(const std::vector<FileId>& file_ids, program::CompileOptions opts)
{
  auto optsptr = std::make_shared<program::CompileOptions>(opts);

  for (FileId f : file_ids)
  {
    auto tu = std::make_unique<TranslationUnit>();
    tu->compile_options = optsptr;
    tu->sourcefile_id = f;

    pendingData().translation_units.push_back(tu.get());

    m_translationunits.add(std::move(tu));
  }
}

TranslationUnit* Snapshot::findTranslationUnit(File* file) const
{
  return translationUnits().find(FileId(file->id));
}

TranslationUnit* Snapshot::getTranslationUnit(TranslationUnitId id) const
{
  return translationUnits().get(id);
}

const TranslationUnitList& Snapshot::translationUnits() const
{
  return m_translationunits;
}

void Snapshot::addTranslationUnitSerializedAst(TranslationUnit* tu, const std::filesystem::path& astfile)
{
  std::string bytes = readFile(astfile);

  insert_translationunit_ast(*m_database, tu, bytes);
}

/**
 * \brief add information about includes to the snapshot
 * \param includes the list of includes
 * \param tu       the translation unit in which the includes were collected
 */
void Snapshot::addIncludes(const std::vector<Include>& includes, TranslationUnit* tu)
{
  std::vector<Include>& list = pendingData().includes[tu];
  list.insert(list.end(), includes.begin(), includes.end());
}

/**
 * \brief returns the list of files included in a given file
 * \param f the id of the file which #include are listed
 */
std::vector<Include> Snapshot::listIncludesInFile(FileId f) const
{
  return select_from_include(*m_database, f, FileId());
}

/**
 * \brief returns the list of files in which a file is included
 * \param f the id of the file that is included
 */
std::vector<Include> Snapshot::findWhereFileIsIncluded(FileId f) const
{
  return select_from_include(*m_database, FileId(), f);
}

void Snapshot::addSymbols(const std::vector<std::shared_ptr<Symbol>>& symbols)
{
  auto& pending_list = pendingData().symbols;
  pending_list.insert(pending_list.end(), symbols.begin(), symbols.end());

  symbolCache().insert(symbols.begin(), symbols.end());
}

/**
 * \brief retrieves a list of symbols
 * \param ids  the ids of the symbol to retrieve
 * 
 * If the requested symbols are no in the cache, they are loaded from the database.
 */
std::map<SymbolId, std::shared_ptr<Symbol>> Snapshot::loadSymbols(const std::set<SymbolId>& ids)
{
  std::map<SymbolId, std::shared_ptr<Symbol>> r;
  loadSymbols(ids, r);
  return r;
}

/**
 * \brief retrieves a list of symbols
 * \param ids     ids of the symbol to retrieve
 * \param outmap  the output map in which the results are written
 * \return the pair (number of symbol loaded from the database, number of symbol found in the cache)
 */
std::pair<size_t, size_t> Snapshot::loadSymbols(const std::set<SymbolId>& ids, std::map<SymbolId, std::shared_ptr<Symbol>>& outmap)
{
  SymbolLoader loader{ *m_database };

  size_t loaded = 0;
  size_t cache = 0;

  for (SymbolId id : ids)
  {
    if (std::shared_ptr<Symbol> symbol = symbolCache().find(id))
    {
      ++cache;
      outmap[symbol->id] = symbol;
    }
    else if(loader.read(id))
    {
      ++loaded;
      outmap[id] = std::make_shared<Symbol>(std::move(loader.symbol));
    }
  }

  return std::make_pair(loaded, cache);
}

void Snapshot::addSymbolReferences(const std::vector<SymbolReference>& list)
{
  auto& pending_list = pendingData().symbol_references;
  pending_list.insert(pending_list.end(), list.begin(), list.end());
}

std::vector<SymbolReference> Snapshot::listReferencesInFile(FileId file)
{
  return select_symbolreference(*m_database, file);
}

SymbolCache& Snapshot::symbolCache()
{
  return m_symbol_cache;
}

/**
 * \brief adds a list of base classes for a symbol
 */
void Snapshot::addBases(SymbolId symid, const std::vector<BaseClass>& bases)
{
  if (symid.valid())
  {
    std::vector<BaseClass>& list = pendingData().bases[symid];
    list.insert(list.end(), bases.begin(), bases.end());
  }
}

bool Snapshot::hasPendingData() const
{
  return m_pending_data != nullptr;
}

void Snapshot::writePendingData()
{
  if (!hasPendingData())
    return;

  sql::Transaction transaction{ *m_database };

  for (const std::pair<const std::string, std::string>& p : m_pending_data->properties)
  {
    insert_info(*m_database, p.first, p.second);
  }

  for (FileId f : m_pending_data->files)
  {
    insert_file(*m_database, *m_files.get(f));
  }

  insert_translationunit(*m_database, m_pending_data->translation_units);

  for (const std::pair<TranslationUnit* const, std::vector<Include>>& p : m_pending_data->includes)
  {
    if (p.first)
    {
      insert_ppinclude(*m_database, *p.first, p.second);
    }

    insert_includes(*m_database, p.second);
  }

  insert_symbol(*m_database, m_pending_data->symbols);

  insert_base(*m_database, m_pending_data->bases);

  insert_symbol_references(*m_database, m_pending_data->symbol_references);

  m_pending_data.reset();
}

PendingData& Snapshot::pendingData()
{
  if (!m_pending_data)
    m_pending_data = std::make_unique<PendingData>();
  return *m_pending_data;
}

} // namespace csnap
