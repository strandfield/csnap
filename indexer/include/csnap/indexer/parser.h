// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_PARSER_H
#define CSNAP_PARSER_H

#include "parsingresult.h"
#include "queue.h"
#include "threadpool.h"

#include <csnap/model/filelist.h>
#include <csnap/model/translationunit.h>

#include <libclang-utils/clang-index.h>

#include <memory>

namespace csnap
{

/**
 * \brief stores parsing results
 * 
 * \sa SharedQueue.
 */
class ParsingResultQueue : public SharedQueue<TranslationUnitParsingResult>
{
public:
  using SharedQueue<TranslationUnitParsingResult>::SharedQueue;
};

/**
 * \brief class that parses translation units
 */
class Parser
{
public:
  Parser(libclang::Index& index, const FileList& flist);
  ~Parser();

  void setThreadCount(size_t n);

  void asyncParse(TranslationUnit* tu);

  bool done() const;
  ParsingResultQueue& results();

private:
  libclang::Index& m_index;
  const FileList& m_files;
  std::unique_ptr<ParsingResultQueue> m_result_queue;
  ThreadPool m_threads;
};

} // namespace csnap

#endif // CSNAP_PARSER_H
