// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "scanner.h"

#include "aggregator.h"
#include "indexer.h"
#include "parser.h"
#include "sln.h"

namespace csnap
{

void save_to_db(TranslationUnitParsingResult& parsingResult, Snapshot& snapshot)
{
  auto path = std::filesystem::temp_directory_path() / (std::to_string(parsingResult.source->id.value()) + ".cxxtranslationunit");

  parsingResult.result->saveTranslationUnit(path.string());

  snapshot.addTranslationUnitSerializedAst(parsingResult.source, path);

  std::filesystem::remove(path);
}

void process_indexing_result(IndexingResult& idxres, IndexingResultAggregator& aggregator)
{
  aggregator.reduce(idxres.references);

  Snapshot& snapshot = aggregator.snapshot();

  for (std::unique_ptr<File>& f : idxres.files)
  {
    snapshot.addFile(std::move(f));
  }

  idxres.files.clear();

  snapshot.addIncludes(idxres.includes, idxres.source);

  snapshot.addSymbols(idxres.symbols);
  
  for (const std::pair<const SymbolId, std::vector<BaseClass>>& p : idxres.bases)
  {
    snapshot.addBases(p.first, p.second);
  }

  snapshot.addSymbolReferences(idxres.references);

  snapshot.writePendingData();
}

/**
 * \brief creates an empty snapshot
 * \param p  the path of the database
 */
void Scanner::initSnapshot(std::filesystem::path& p)
{
  m_snapshot = std::make_unique<Snapshot>(Snapshot::create(p));
}

/**
 * \brief fills the snapshot by scanning a Visual Studio solution
 * \param slnPath  path to the sln file
 * 
 * \warning initSnapshot() must be called before calling this function.
 */
void Scanner::scanSln(const std::filesystem::path& slnPath)
{
  openSln(slnPath, *m_snapshot);

  m_snapshot->writePendingData();

  libclang::LibClang clang;
  libclang::Index index = clang.createIndex();

  Parser parser{ index, m_snapshot->files() };
  parser.setThreadCount(this->nb_parsing_threads);

  for (TranslationUnit* tu : m_snapshot->translationUnits().all())
  {
    parser.asyncParse(tu);
  }

  // We have some time before parsing results become available, 
  // we use this time to save the file's content into database:
  m_snapshot->addFilesContent();

  Indexer indexer{ index, *m_snapshot };
  IndexingResultAggregator aggregator{ *m_snapshot };

  while (!parser.done() || !parser.results().empty())
  {
    TranslationUnitParsingResult pr{ parser.results().next() };

    if (pr.result)
    {
      if (save_ast)
      {
        save_to_db(pr, *m_snapshot);
      }

      indexer.asyncIndex(std::move(pr));
    }

    while (!indexer.results().empty())
    {
      IndexingResult idxres{ indexer.results().next() };
      process_indexing_result(idxres, aggregator);
    }
  }

  while (!indexer.done() || !indexer.results().empty())
  {
    IndexingResult idxres{ indexer.results().next() };
    process_indexing_result(idxres, aggregator);
  }
}

} // namespace csnap
