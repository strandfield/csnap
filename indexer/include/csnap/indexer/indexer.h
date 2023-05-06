// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_INDEXER_H
#define CSNAP_INDEXER_H

#include "parsingresult.h"
#include "queue.h"
#include "threadpool.h"

#include <csnap/model/filelist.h>
#include <csnap/model/include.h>
#include <csnap/model/reference.h>
#include <csnap/model/usrmap.h>

#include <libclang-utils/clang-index.h>
#include <libclang-utils/index-action.h>

#include <chrono>
#include <map>
#include <memory>
#include <vector>

namespace csnap
{

struct BaseClass;
struct Symbol;

class Snapshot;

struct IndexingResult
{
  TranslationUnit* source = nullptr;
  std::chrono::milliseconds indexing_time;

  /**
   * \brief the list of files added to the snapshot while indexing the translation unit 
   */
  std::vector<std::unique_ptr<File>> files;

  /**
   * \brief the list of files included in the translation unit
   */
  std::vector<Include> includes;

  /**
   * \brief the list of symbols that were first encountered while indexing the translation unit
   */
  std::vector<std::shared_ptr<Symbol>> symbols;

  /**
   * \brief the list of symbol references collected when indexing the translation unit
   */
  std::vector<SymbolReference> references;

  /**
   * \brief the list of bases of classes first encountered while indexing the translation unit
   */
  std::map<SymbolId, std::vector<BaseClass>> bases;
};

using IndexerResultQueue = SharedQueue<IndexingResult>;

/**
 * \brief perform the indexing of the parsed translation units
 * 
 * This class is used to index the translation units parsed by the Parser class.
 */
class Indexer
{
public:
  Indexer(libclang::Index& index, Snapshot& snapshot);
  ~Indexer();

  libclang::LibClang& libclangAPI();
  libclang::IndexAction& indexAction();

  Snapshot& snapshot() const;

  void asyncIndex(TranslationUnitParsingResult parsingResult);

  bool done() const;

  IndexerResultQueue& results();

  std::pair<File*, std::unique_ptr<File>> getFile(std::string path);

  GlobalUsrMap& sharedUsrMap();

public:
  /**
   * \brief whether previously unknown files encountered while indexing should be indexed 
   * 
   * While indexing, all files included with an #include preprocessor directive are traversed.
   * These files may not have been listed in previous steps by the scanner (e.g., because 
   * they are not part of the project but are rather external dependencies).
   * That is often the case for system and standard library headers.
   * 
   * If \a collect_new_files is true (it is false by default), these files are added to 
   * the snapshot and indexed just like the project files; otherwise they are mostly ignored,
   * only the symbol that are actually used in the project are indexed.
   */
  bool collect_new_files = false;

private:
  libclang::Index& m_index;
  Snapshot& m_snapshot;
  std::unique_ptr<libclang::IndexAction> m_index_action;
  IndexerResultQueue m_results;
  ThreadPool m_threads;
  GlobalUsrMap m_usrs;
  int m_file_id_generator = -1; // used only if collect_new_files is true
};

} // namespace csnap

#endif // CSNAP_INDEXER_H
