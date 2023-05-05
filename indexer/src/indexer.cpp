// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "indexer.h"

#include "csnap/database/snapshot.h"

#include "csnap/model/file.h"
#include "csnap/model/reference.h"
#include "csnap/model/symbol.h"
#include "csnap/model/usrmap.h"

#include <libclang-utils/index-action.h>
#include <libclang-utils/clang-cursor.h>

#include <cassert>
#include <filesystem>
#include <iostream>

namespace csnap
{

class TranslationUnitIndexer : public libclang::BasicIndexer
{
private:
  csnap::Indexer& indexer;
  UsrMap usrs;
  std::map<std::string, std::shared_ptr<Symbol>> symbols;

public:
  IndexingResult result;

public:
  TranslationUnitIndexer(csnap::Indexer& idx, TranslationUnit* tu) : libclang::BasicIndexer(idx.libclangAPI()),
    indexer(idx),
    usrs(idx.sharedUsrMap().clone())
  {
    result.source = tu;
  }

  CXIdxClientContainer startedTranslationUnit()
  {
    return nullptr;
  }

  void* enteredMainFile(const libclang::File& mainFile)
  {
    std::string path = mainFile.getFileName();
    auto [rawptr, owningptr] = indexer.getFile(path);

    // the file should already exist in the snapshot
    assert(owningptr == nullptr);

    if (owningptr)
    {
      rawptr = owningptr.get();
      result.files.push_back(std::move(owningptr));
    }

    return rawptr;
  }

  void* ppIncludedFile(const CXIdxIncludedFileInfo* inclFile)
  {
    // ppIncludedFile() is called for every #include in the translation unit
    // (including nested #includes).
    // We collect the following information here:
    // - the id of the included file (which we may need to create if we never encountered the file);
    // - the id of the file in which the #include is located;
    // - the line number of the #include.
    // The id of the included file is returned by this function so that 
    // it can be attached to the file by libclang and later be retrieved using getClientData().
    // Note that all calls to ppIncludedFile() seem to happen before any calls to 
    // indexDeclaration() or indexEntityReference() so this function cannot be used 
    // to know in which file "we currently are". 

    std::string path = indexer.libclangAPI().file(inclFile->file).getFileName();

    auto [rawptr, owningptr] = indexer.getFile(path);

    if (owningptr)
    {
      rawptr = owningptr.get();
      result.files.push_back(std::move(owningptr));
    }

    if (!rawptr)
      return nullptr; // $TODO: we should find a way to log the include, for completeness

    FileLocation loc = getFileLocation(inclFile->hashLoc);

    Include inc;
    inc.included_file_id = rawptr->id;
    inc.line = loc.line;

    if (loc.client_data)
      inc.file_id = reinterpret_cast<File*>(loc.client_data)->id;

    if (inc.file_id.valid())
    {
      result.includes.push_back(inc);
    }
    else
    {
      std::cerr << "could not get id for " << libclangAPI().file(loc.file).getFileName() << std::endl;
    }

    return rawptr;
  }

  void indexDeclaration(const CXIdxDeclInfo* decl)
  {
    FileLocation loc = getFileLocation(decl->loc);

    if (!loc.client_data)
    {
      // the declaration belongs to a file that is skipped
      return;
    }

    Symbol* symbol = get_symbol(decl);

    if (!symbol)
      return;

    setClientData(decl->declAsContainer, symbol);
    setClientData(decl->entityInfo, symbol);

    // It seems indexEntityReference() is not called for declaration despite the 
    // CXSymbolRole enum suggesting it could; so we create the reference manually here.
    if(loc.client_data)
    {
      FileId fileid = reinterpret_cast<File*>(loc.client_data)->id;

      SymbolReference symref;
      symref.file_id = fileid;
      symref.col = loc.column;
      symref.line = loc.line;
      symref.symbol_id = symbol->id;

      symref.flags = decl->isDefinition ? CXSymbolRole_Definition : CXSymbolRole_Declaration;

      if (decl->isImplicit)
        symref.flags |= CXSymbolRole_Implicit;

      if (void* cdata = getClientData(decl->semanticContainer))
        symref.parent_symbol_id = reinterpret_cast<Symbol*>(cdata)->id;

      result.references.push_back(symref);
    }
  }

  void indexEntityReference(const CXIdxEntityRefInfo* ref)
  {
    FileLocation loc = getFileLocation(ref->loc);

    if (!loc.client_data)
    {
      // the entity reference belongs to a file that is skipped
      return;
    }

    FileId fileid = reinterpret_cast<File*>(loc.client_data)->id;

    Symbol* symbol = get_symbol(ref->referencedEntity);

    if (!symbol)
      return;

    SymbolReference symref;
    symref.file_id = fileid;
    symref.col = loc.column;
    symref.line = loc.line;
    symref.symbol_id = symbol->id;
    symref.flags = ref->role;

    if (ref->parentEntity)
    {
      if (Symbol* parent_symbol = get_symbol(ref->parentEntity))
        symref.parent_symbol_id = parent_symbol->id;
    }

    result.references.push_back(symref);
  }

protected:

  Symbol* lookup_symbol(const std::string& usr) const
  {
    auto it = symbols.find(usr);
    return it != symbols.end() ? it->second.get() : nullptr;
  }

  Symbol* insert_placeholder_symbol(SymbolId id, std::string usr)
  {
    auto symbol = std::make_shared<Symbol>();
    symbol->id = id;
    symbols[usr] = symbol;
    symbol->usr = std::move(usr);
    return symbol.get();
  }

  Symbol* get_symbol(const CXIdxDeclInfo* decl)
  {
    std::string usr{ decl->entityInfo->USR };

    if (Symbol* symbol = lookup_symbol(usr))
    {
      if (decl->isDefinition)
      {
        // we may more accurately fill the Symbol struct here

        if (decl->entityInfo->kind == CXIdxEntity_CXXClass)
        {
          list_bases(symbol->id, decl);
        }

        fill_symbol(*symbol, libclangAPI().cursor(decl->cursor));
      }

      return symbol;
    }

    SymbolId id = usrs.find(usr);

    if (id.valid())
    {
      return insert_placeholder_symbol(id, std::move(usr));
    }

    // $TODO: maybe we should assign temporary ids to 
    // the symbols and then rewrite the id when aggregating 
    // the indexing results.
    auto [uid, inserted] = indexer.sharedUsrMap().get(usr);
    id = uid;
    // update local "cache"
    usrs.insert(usr, id);

    if (!inserted)
      return insert_placeholder_symbol(id, std::move(usr));

    // The SymbolId was just created, we need to create and fill the corresponding Symbol struct.

    std::shared_ptr<Symbol> sym = create_symbol(decl, id);

    if (sym->kind == Whatsit::CXXClass)
    {
      list_bases(*sym, decl);
    }

    fill_symbol(*sym, libclangAPI().cursor(decl->cursor));

    symbols[usr] = sym;
    result.symbols.push_back(sym);

    return sym.get();
  }

  Symbol* get_symbol(const CXIdxEntityInfo* info)
  {
    if (void* cdata = getClientData(info))
      return reinterpret_cast<Symbol*>(cdata);

    std::string usr{ info->USR };

    if (Symbol* symbol = lookup_symbol(usr))
      return symbol;

    SymbolId id = usrs.find(usr);

    if (id.valid())
      return insert_placeholder_symbol(id, std::move(usr));

    auto [uid, inserted] = indexer.sharedUsrMap().get(usr);
    id = uid;
    // update local "cache"
    usrs.insert(usr, id);

    if (!inserted)
      return insert_placeholder_symbol(id, std::move(usr));

    // The SymbolId was just created, we need to create and fill the corresponding Symbol struct.

    std::shared_ptr<Symbol> sym = create_symbol(info, id);

    symbols[usr] = sym;
    result.symbols.push_back(sym);

    return sym.get();
  }

private:

  SymbolId get_symbol_id(const libclang::Cursor& cursor)
  {
    if (cursor.isNull())
      return SymbolId();

    static const std::map<CXCursorKind, CXIdxEntityKind> dict = {
      { CXCursor_TypedefDecl,        CXIdxEntity_Typedef },
      { CXCursor_FunctionDecl,       CXIdxEntity_Function },
      { CXCursor_VarDecl,            CXIdxEntity_Variable },
      { CXCursor_EnumConstantDecl,   CXIdxEntity_EnumConstant },
      { CXCursor_EnumDecl,           CXIdxEntity_Enum },
      { CXCursor_StructDecl,         CXIdxEntity_Struct },
      { CXCursor_UnionDecl,          CXIdxEntity_Union },
      { CXCursor_ClassDecl,          CXIdxEntity_CXXClass },
      { CXCursor_Namespace,          CXIdxEntity_CXXNamespace },
      { CXCursor_NamespaceAlias,     CXIdxEntity_CXXNamespaceAlias },
      { CXCursor_Constructor,        CXIdxEntity_CXXConstructor },
      { CXCursor_Destructor,         CXIdxEntity_CXXDestructor },
      { CXCursor_ConversionFunction, CXIdxEntity_CXXConversionFunction },
      { CXCursor_TypeAliasDecl,      CXIdxEntity_CXXTypeAlias },
    };

    auto it = dict.find(cursor.kind());

    if (it == dict.end())
      return SymbolId();

    std::string usr{ cursor.getUSR() };

    if (usr.empty())
      return SymbolId();

    SymbolId id = usrs.find(usr);

    if (id.valid())
      return id;

    auto [uid, inserted] = indexer.sharedUsrMap().get(usr);
    id = uid;
    // update local "cache"
    usrs.insert(usr, id);

    if (!inserted)
      return id;

    // create symbol
    std::shared_ptr<Symbol> sym = create_symbol(cursor, id, static_cast<Whatsit>(it->second));
    symbols[usr] = sym;
    result.symbols.push_back(sym);

    return id;
  }

  SymbolId get_parent_symbol_id(const CXIdxEntityInfo* info)
  {
    libclang::Cursor c = libclangAPI().cursor(info->cursor);
    c = c.getSemanticParent();
    return get_symbol_id(c);
  }

  const char* name(const CXIdxEntityInfo* entity)
  {
    return entity->name != nullptr ? entity->name : "";
  }

  void fill_symbol(Symbol& s, const libclang::Cursor& c)
  {
    switch (c.kind())
    {
    case CXCursor_EnumDecl:
    {
      csnap::set_flag(s, Symbol::IsScoped, c.EnumDecl_isScoped());
    }
    break;
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    {
      csnap::set_flag(s, Symbol::Default, c.CXXMethod_isDefaulted());
      csnap::set_flag(s, Symbol::Const, c.CXXMethod_isConst());
      csnap::set_flag(s, Symbol::Static, c.CXXMethod_isStatic());
      csnap::set_flag(s, Symbol::Virtual, c.CXXMethod_isVirtual());
      csnap::set_flag(s, Symbol::Pure, c.CXXMethod_isPureVirtual());
    }
    default:
      break;
    }

  }

  std::shared_ptr<Symbol> create_symbol(const libclang::Cursor& cursor, SymbolId id, Whatsit what, SymbolId parent_id)
  {
    auto s = std::make_unique<Symbol>(what, cursor.getSpelling());
    s->display_name = cursor.getDisplayName();
    s->usr = cursor.getUSR();

    fill_symbol(*s, cursor);

    s->id = id;
    s->parent_id = parent_id;

    return s;
  }

  std::shared_ptr<Symbol> create_symbol(const libclang::Cursor& cursor, SymbolId id, Whatsit what)
  {
    SymbolId parent_id = get_symbol_id(cursor.getSemanticParent());
    return create_symbol(cursor, id, what, parent_id);
  }

  std::shared_ptr<Symbol> create_symbol(const CXIdxEntityInfo* info, SymbolId id, SymbolId parent_id)
  {
    auto s = std::make_unique<Symbol>(static_cast<Whatsit>(info->kind), name(info));
    s->display_name = libclangAPI().cursor(info->cursor).getDisplayName();
    s->usr = info->USR;

    s->id = id;
    s->parent_id = parent_id;

    fill_symbol(*s, libclangAPI().cursor(info->cursor));

    return s;
  }

  std::shared_ptr<Symbol> create_symbol(const CXIdxEntityInfo* info, SymbolId id)
  {
    SymbolId parent_id = get_parent_symbol_id(info);
    return create_symbol(info, id, parent_id);
  }

  std::shared_ptr<Symbol> create_symbol(const CXIdxDeclInfo* decl, SymbolId id)
  {
    SymbolId parent_id;
    
    if (void* cdata = getClientData(decl->semanticContainer))
      parent_id = reinterpret_cast<Symbol*>(cdata)->id;

    if (parent_id.valid())
      return create_symbol(decl->entityInfo, id, parent_id);
    else
      return create_symbol(decl->entityInfo, id);
  }

  void list_bases(SymbolId symbol_id, const CXIdxDeclInfo* decl)
  {
    const CXIdxCXXClassDeclInfo* classdecl = getCXXClassDeclInfo(decl);

    if (!classdecl)
      return;

    std::vector<BaseClass> bases;

    for (unsigned int i(0); i < classdecl->numBases; ++i)
    {
      const CXIdxBaseClassInfo* base = classdecl->bases[i];

      if (void* cdata = getClientData(base->base))
      {
        BaseClass b;
        b.base_id = reinterpret_cast<Symbol*>(cdata)->id;
        b.access_specifier = static_cast<csnap::AccessSpecifier>(libclangAPI().cursor(base->cursor).getCXXAccessSpecifier());
        bases.push_back(b);
      }
    }

    if (!bases.empty())
    {
      result.bases[symbol_id] = std::move(bases);
    }
  }

  void list_bases(const Symbol& symbol, const CXIdxDeclInfo* decl)
  {
    list_bases(symbol.id, decl);
  }
};

class IndexTranslationUnit : public Runnable
{
public:
  Indexer& indexer;
  TranslationUnitParsingResult parsingResult;

public:
  IndexTranslationUnit(Indexer& idxr, TranslationUnitParsingResult pr) :
    indexer(idxr),
    parsingResult(std::move(pr))
  {
    m_task_number = ++m_task_counter;
  }

  void run() override
  {
    const File* sourcefile = indexer.snapshot().files().get(parsingResult.source->sourcefile_id);
    std::cout << "[" << m_task_number << "/" << indexer.snapshot().translationUnits().count() << "] " << sourcefile->path << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    TranslationUnitIndexer tui{ indexer, parsingResult.source };
    indexer.indexAction().indexTranslationUnit(*parsingResult.result, tui);

    auto end = std::chrono::high_resolution_clock::now();
    tui.result.indexing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    indexer.results().write(std::move(tui.result));
  }

private:
  static size_t m_task_counter;
  size_t m_task_number;
};

size_t IndexTranslationUnit::m_task_counter = 0;

Indexer::Indexer(libclang::Index& index, Snapshot& snapshot) :
  m_index(index),
  m_snapshot(snapshot),
  m_threads(1), // $Note: not sure clang index action is thread-safe so we only use 1 thread for indexing
  m_file_id_generator((int)snapshot.files().all().size())
{
  m_index_action = std::make_unique<libclang::IndexAction>(index);
}

Indexer::~Indexer()
{
  if (!results().empty())
  {
    std::cout << "Warning: results() isn't empty in ~Indexer()" << std::endl;
  }
}

libclang::LibClang& Indexer::libclangAPI()
{
  return indexAction().api;
}

libclang::IndexAction& Indexer::indexAction()
{
  return *m_index_action;
}

/**
 * \brief returns a reference to the snapshot passed to the constructor
 */
Snapshot& Indexer::snapshot() const
{
  return m_snapshot;
}

/**
 * \brief starts the indexing of a translation unit asynchronously
 * \param parsingResult  parsing result as produced by the Parser class
 * 
 * Please note that although indexing is performed asynchronously, it is 
 * not really "multi-threaded": only a sinlge thread performs the indexing.
 * This means that the indexing of @a parsingResult will only start after 
 * all previous calls to asyncIndex() have completed.
 */
void Indexer::asyncIndex(TranslationUnitParsingResult parsingResult)
{
  if (!parsingResult.result)
    return;

  m_threads.run(new IndexTranslationUnit(*this, std::move(parsingResult)));
}

/**
 * \brief returns whether all indexing tasks have been completed
 */
bool Indexer::done() const
{
  return m_threads.done();
}

/**
 * \brief returns the indexer results queue
 */
IndexerResultQueue& Indexer::results()
{
  return m_results;
}

/**
 * \brief returns a pointer to a File matching the given path
 * \param path the path of the file
 * 
 * If the file currently does not exist in the snapshot and \a collect_new_files 
 * is true, a new File object is created ; otherwise, this function returns nullptr.
 * 
 * This function returns a pair of pointers, a non-owning and an owning one;
 * only one of which is not null, depending on whether a File object was 
 * actually created by this call.
 */
std::pair<File*, std::unique_ptr<File>> Indexer::getFile(std::string path)
{
  // $TODO: make thread-safe

  File* f = snapshot().findFile(path);

  if (f || !collect_new_files)
    return { f, nullptr };

  f = new File;
  f->path = std::move(path);
  f->id = FileId(m_file_id_generator++);

  return { nullptr, std::unique_ptr<File>(f) };
}

/**
 * \brief a usr map shared among all indexing tasks
 * 
 * USRs (Unified Symbol Resolution) are used to match symbols across translation units.
 * This map is used to always assign the same id to a symbol across all indexing tasks.
 */
GlobalUsrMap& Indexer::sharedUsrMap()
{
  return m_usrs;
}

} // namespace csnap
