// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_INDEXER_H
#define CSNAP_INDEXER_H

#include "parsingresult.h"
#include "queue.h"
#include "threadpool.h"

#include <csnap/model/filelist.h>
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
  std::vector<File*> files; // $TODO: change to unique_ptr?

  /**
   * \brief the list of files included in the translation unit
   */
  std::vector<File*> included_files;

  // $TODO: use findIncludesInFile() to list #includes for each file
  std::map<FileId, std::vector<File*>> per_file_includes;

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

  // $TODO: return a pair owning / non-owning
  std::pair<File*, bool> getOrCreateFile(std::string path);

  GlobalUsrMap& sharedUsrMap();

private:
  libclang::Index& m_index;
  Snapshot& m_snapshot;
  std::unique_ptr<libclang::IndexAction> m_index_action;
  IndexerResultQueue m_results;
  ThreadPool m_threads;
  GlobalUsrMap m_usrs;
  int m_file_id_generator = -1;
};

} // namespace csnap

#endif // CSNAP_INDEXER_H
