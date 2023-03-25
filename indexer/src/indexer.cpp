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
  UsrMap usrs;
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
    auto [rawptr, owningptr] = indexer.getOrCreateFile(path);

    if (owningptr)
    {
      rawptr = owningptr.get();
      result.files.push_back(std::move(owningptr));
    }

    return m_files.create(rawptr->id);
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

    std::string path = indexer.libclangAPI().file(inclFile->file).getFileName();

    auto [rawptr, owningptr] = indexer.getOrCreateFile(path);

    if (owningptr)
    {
      rawptr = owningptr.get();
      result.files.push_back(std::move(owningptr));
    }

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
    std::string usr{ decl->entityInfo->USR };

    SymbolId id = usrs.find(usr);

    if (!id.valid())
    {
      // $TODO: maybe we should assign temporary ids to 
      // the symbols and then rewrite the id when aggregating 
      // the indexing results.
      auto [uid, inserted] = indexer.sharedUsrMap().get(usr);
      id = uid;
      // update local "cache"
      usrs.insert(usr, id);

      if (inserted)
      {
        std::unique_ptr<Symbol> sym = create_symbol(decl, id);

        if (sym->kind == Whatsit::CXXClass)
        {
          list_bases(*sym, decl);
        }

        result.symbols.push_back(std::move(sym));
      }
    }
    else
    {
      if (decl->isDefinition || decl->isRedeclaration)
      {
        // $TODO: we may more accurately fill the Symbol struct here ?
      }
    }

    void* refid = m_symbols.create(id);
    setClientData(decl->declAsContainer, refid);
    setClientData(decl->entityInfo, refid);
  }

  void indexEntityReference(const CXIdxEntityRefInfo* ref)
  {
    SymbolId symid = m_symbols.get(getClientData(ref->referencedEntity));

    if (!symid.valid())
    {
      // $TODO: what?
      //std::string usr{ ref->referencedEntity->USR };
      //usrs.find(usr);

      return;
    }
 
    FileLocation loc = getFileLocation(ref->loc);
    FileId fileid = m_files.get(loc.client_data);

    if (!fileid.valid())
      return;

    SymbolReference symref;
    symref.col = loc.column;
    symref.line = loc.line;
    symref.file_id = fileid.value();
    symref.symbol_id = symid.value();
    symref.flags = ref->role;
    result.references.push_back(symref);
  }

protected:

  const char* name(const CXIdxEntityInfo* entity)
  {
    return entity->name != nullptr ? entity->name : "";
  }

  std::unique_ptr<Symbol> create_symbol(const CXIdxDeclInfo* decl, SymbolId id)
  {
    auto s = std::make_unique<Symbol>(static_cast<Whatsit>(decl->entityInfo->kind), name(decl->entityInfo));
    s->id = SymbolId(id.value());
    s->parent_id = m_symbols.get(getClientData(decl->semanticContainer));
    s->display_name = libclangAPI().cursor(decl->entityInfo->cursor).getDisplayName();
    s->usr = decl->entityInfo->USR;

    // $TODO: fill extra information depending on the kind of symbol

    return s;
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
 * \brief returns a pointer to a File, creating it if missing
 * 
 * This function returns a pair of pointers, a non-owning and an owning one;
 * only one of which is not null, depending on whether a File object was 
 * actually created by this call.
 */
std::pair<File*, std::unique_ptr<File>> Indexer::getOrCreateFile(std::string path)
{
  // $TODO: make thread-safe

  File* f = snapshot().findFile(path);

  if (f)
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
