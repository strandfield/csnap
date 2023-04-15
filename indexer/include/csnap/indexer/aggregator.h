// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_AGGREGATOR_H
#define CSNAP_AGGREGATOR_H

#include "indexer.h"

#include <map>

namespace csnap
{

class Snapshot;

/**
 * \brief helper class for aggregating indexing results
 * 
 * The indexing of a translation unit may produce results that have already been 
 * produced while indexing another translation unit (e.g., because both translation 
 * units included the same file).
 * 
 * The reduce() function in this class removes symbol references that are already 
 * known so that no duplicates end up in the database.
 */
class IndexingResultAggregator
{
public:
  explicit IndexingResultAggregator(Snapshot& s);

  Snapshot& snapshot() const;

  void reduce(std::vector<SymbolReference>& references);

private:
  Snapshot& m_snapshot;

  struct ReferencesInFileInfo
  {
    size_t num_references = 0;
  };

  std::map<FileId, ReferencesInFileInfo> m_files_data;
};

} // namespace csnap

#endif // CSNAP_AGGREGATOR_H
