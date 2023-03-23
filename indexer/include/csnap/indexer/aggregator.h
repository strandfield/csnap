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
