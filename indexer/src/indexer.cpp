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

template<typename T>
class AddressableIds
{
public:

  void* create(Identifier<T> id)
  {
    m_ids.push_back(std::make_unique<Identifier<T>>(id));
    return m_ids.back().get();
  }

  Identifier<T> get(void* data)
  {
    auto* ptr = reinterpret_cast<Identifier<T>*>(data);
    return ptr ? *ptr : Identifier<T>();
  }

private:
  std::vector<std::unique_ptr<Identifier<T>>> m_ids;
};

class TranslationUnitIndexer : public libclang::BasicIndexer
{
private:
  csnap::Indexer& indexer;
  UsrMap usrs; // $TODO: build this local cache in the constructor
  AddressableIds<Symbol> m_symbols;
  AddressableIds<File> m_files;

public:
  IndexingResult result;

public:
  TranslationUnitIndexer(csnap::Indexer& idx, TranslationUnit* tu) : libclang::BasicIndexer(idx.libclangAPI()),
    indexer(idx)
  {
    result.source = tu;
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

    return rawptr ? m_files.create(rawptr->id) : nullptr;
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
    inc.file_id = m_files.get(loc.client_data);

    if (inc.file_id.valid())
    {
      result.includes.push_back(inc);
    }
    else
    {
      std::cerr << "oups" << std::endl;
    }

    return m_files.create(rawptr->id);
  }

  void indexDeclaration(const CXIdxDeclInfo* decl)
  {
    FileLocation loc = getFileLocation(decl->loc);

    if (!loc.client_data)
    {
      // the declaration belongs to a file that is skipped
      return;
    }

    SymbolId id = get_symbol_id(decl);

    if (assert(id.valid()), !id.valid())
      return;

    void* refid = m_symbols.create(id);
    setClientData(decl->declAsContainer, refid);
    setClientData(decl->entityInfo, refid);
  }

  void indexEntityReference(const CXIdxEntityRefInfo* ref)
  {
    FileLocation loc = getFileLocation(ref->loc);

    if (!loc.client_data)
    {
      // the entity reference belongs to a file that is skipped
      return;
    }

    FileId fileid = m_files.get(loc.client_data);
    assert(fileid.valid());
    if (!fileid.valid())
      return;

    SymbolId symid = get_symbol_id(ref->referencedEntity);

    if (assert(symid.valid()), !symid.valid())
      return;

    SymbolReference symref;
    symref.file_id = fileid.value();
    symref.col = loc.column;
    symref.line = loc.line;
    symref.symbol_id = symid.value();
    symref.flags = ref->role;

    if(ref->parentEntity) 
      symref.parent_symbol_id = get_symbol_id(ref->parentEntity);

    result.references.push_back(symref);
  }

protected:

  SymbolId get_symbol_id(const CXIdxDeclInfo* decl)
  {
    std::string usr{ decl->entityInfo->USR };

    SymbolId id = usrs.find(usr);

    if (id.valid())
    {
      if (decl->isDefinition || decl->isRedeclaration)
      {
        // $TODO: we may more accurately fill the Symbol struct here ?
      }

      return id;
    }

    // $TODO: maybe we should assign temporary ids to 
    // the symbols and then rewrite the id when aggregating 
    // the indexing results.
    auto [uid, inserted] = indexer.sharedUsrMap().get(usr);
    id = uid;
    // update local "cache"
    usrs.insert(usr, id);

    if (!inserted)
      return id;

    // The SymbolId was just created, we need to create the corresponding Symbol struct.

    std::unique_ptr<Symbol> sym = create_symbol(decl, id);

    if (sym->kind == Whatsit::CXXClass)
    {
      list_bases(*sym, decl);
    }

    result.symbols.push_back(std::move(sym));

    return id;
  }

  SymbolId get_symbol_id(const CXIdxEntityInfo* info)
  {
    SymbolId id = m_symbols.get(getClientData(info));

    if (id.valid())
      return id;

    std::string usr{ info->USR };

    id = usrs.find(usr);

    if (id.valid())
      return id;

    auto [uid, inserted] = indexer.sharedUsrMap().get(usr);
    id = uid;
    // update local "cache"
    usrs.insert(usr, id);

    if (!inserted)
      return id;

    // The SymbolId was just created, we need to create the corresponding Symbol struct.

    std::unique_ptr<Symbol> sym = create_symbol(info, id);

    result.symbols.push_back(std::move(sym));

    return id;
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
    std::unique_ptr<Symbol> sym = create_symbol(cursor, id, static_cast<Whatsit>(it->second));
    result.symbols.push_back(std::move(sym));

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

  std::unique_ptr<Symbol> create_symbol(const libclang::Cursor& cursor, SymbolId id, Whatsit what, SymbolId parent_id)
  {
    auto s = std::make_unique<Symbol>(what, cursor.getSpelling());
    s->display_name = cursor.getDisplayName();
    s->usr = cursor.getUSR();

    // $TODO: fill extra information depending on the kind of symbol

    s->id = id;
    s->parent_id = parent_id;

    return s;
  }

  std::unique_ptr<Symbol> create_symbol(const libclang::Cursor& cursor, SymbolId id, Whatsit what)
  {
    SymbolId parent_id = get_symbol_id(cursor.getSemanticParent());
    return create_symbol(cursor, id, what, parent_id);
  }

  std::unique_ptr<Symbol> create_symbol(const CXIdxEntityInfo* info, SymbolId id, SymbolId parent_id)
  {
    auto s = std::make_unique<Symbol>(static_cast<Whatsit>(info->kind), name(info));
    s->display_name = libclangAPI().cursor(info->cursor).getDisplayName();
    s->usr = info->USR;

    s->id = id;
    s->parent_id = parent_id;

    // $TODO: fill extra information depending on the kind of symbol

    return s;
  }

  std::unique_ptr<Symbol> create_symbol(const CXIdxEntityInfo* info, SymbolId id)
  {
    SymbolId parent_id = get_parent_symbol_id(info);
    return create_symbol(info, id, parent_id);
  }

  std::unique_ptr<Symbol> create_symbol(const CXIdxDeclInfo* decl, SymbolId id)
  {
    SymbolId parent_id = m_symbols.get(getClientData(decl->semanticContainer));
    if (parent_id.valid())
      return create_symbol(decl->entityInfo, id, parent_id);
    else
      return create_symbol(decl->entityInfo, id);
  }

  void list_bases(const Symbol& symbol, const CXIdxDeclInfo* decl)
  {
    const CXIdxCXXClassDeclInfo* classdecl = getCXXClassDeclInfo(decl);

    if (!classdecl)
      return;

    std::vector<BaseClass> bases;

    for (unsigned int i(0); i < classdecl->numBases; ++i)
    {
      const CXIdxBaseClassInfo* base = classdecl->bases[i];
      
      SymbolId symid = m_symbols.get(getClientData(base->base));

      if (!symid.valid())
      {
        // $TODO: what?
        continue;
      }

      BaseClass b;
      b.base_id = symid;
      b.access_specifier = static_cast<csnap::AccessSpecifier>(libclangAPI().cursor(base->cursor).getCXXAccessSpecifier());
      bases.push_back(b);
    }

    result.bases[symbol.id] = std::move(bases);
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

  }

  void run() override
  {
    auto start = std::chrono::high_resolution_clock::now();

    TranslationUnitIndexer tui{ indexer, parsingResult.source };
    indexer.indexAction().indexTranslationUnit(*parsingResult.result, tui);

    auto end = std::chrono::high_resolution_clock::now();
    tui.result.indexing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    indexer.results().write(std::move(tui.result));
  }
};

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

void Indexer::asyncIndex(TranslationUnitParsingResult parsingResult)
{
  if (!parsingResult.result)
    return;

  m_threads.run(new IndexTranslationUnit(*this, std::move(parsingResult)));
}

bool Indexer::done() const
{
  return m_threads.done();
}

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

GlobalUsrMap& Indexer::sharedUsrMap()
{
  return m_usrs;
}

} // namespace csnap
