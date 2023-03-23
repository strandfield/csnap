// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "parser.h"

#include "csnap/model/file.h"

#include <filesystem>
#include <iostream>

namespace csnap
{

struct ParsingWork
{
  TranslationUnit* source = nullptr;
  std::filesystem::path sourcefile;
};

static TranslationUnitParsingResult parse_translation_unit(libclang::Index& index, const ParsingWork& work)
{
  TranslationUnitParsingResult result;
  result.source = work.source;

  auto start = std::chrono::high_resolution_clock::now();
  result.result = std::make_unique<libclang::TranslationUnit>(index.parseTranslationUnit(work.sourcefile.string(),
    work.source->compile_options->includedirs, 
    CXTranslationUnit_DetailedPreprocessingRecord));
  auto end = std::chrono::high_resolution_clock::now();
  result.parsing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  return result;
}

class ParseTranslationUnit : public Runnable
{
public:
  libclang::Index& index;
  ParsingWork work;
  ParsingResultQueue& results;

public:

  ParseTranslationUnit(libclang::Index& idx, ParsingWork w, ParsingResultQueue& rqueue) :
    index(idx),
    work(std::move(w)),
    results(rqueue)
  {

  }

  void run() override
  {
    TranslationUnitParsingResult result = parse_translation_unit(index, work);
    results.write(std::move(result));
  }
};

Parser::Parser(libclang::Index& index, const FileList& flist) :
  m_index(index),
  m_files(flist),
  m_result_queue(std::make_unique<ParsingResultQueue>()),
  m_threads{1}
{

}

Parser::~Parser()
{
  if (!results().empty())
  {
    std::cout << "Warning: results() isn't empty in ~Parser()" << std::endl;
  }
}

void Parser::asyncParse(TranslationUnit* tu)
{
  if (!tu)
    return;

  std::filesystem::path p = m_files.get(FileId(tu->sourcefile_id))->path;

  ParsingWork w;
  w.source = tu;
  w.sourcefile = p;

  m_threads.run(new ParseTranslationUnit(m_index, std::move(w), *m_result_queue));
}

/**
 * \brief returns whether all asynchronous parsing tasks have been completed
 */
bool Parser::done() const
{
  return m_threads.done();
}

ParsingResultQueue& Parser::results()
{
  return *m_result_queue;
}

} // namespace csnap
