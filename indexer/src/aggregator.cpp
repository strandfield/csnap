// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "aggregator.h"

#include "csnap/database/snapshot.h"

#include <algorithm>
#include <iostream>

namespace csnap
{

/**
 * \brief constructs a result aggregator on the snapshot
 * \param s  the snapshot
 */
IndexingResultAggregator::IndexingResultAggregator(Snapshot& s) :
  m_snapshot(s)
{

}

/**
 * \brief returns the snapshot on which the aggregator operates 
 */
Snapshot& IndexingResultAggregator::snapshot() const
{
  return m_snapshot;
}

/**
 * \brief removes all already references that are already known in the snapshot
 * \param references  a list of references
 */
void IndexingResultAggregator::reduce(std::vector<SymbolReference>& references)
{
  if (references.empty())
    return;

  auto comp = [](const SymbolReference& lhs, const SymbolReference& rhs) {
    return std::make_tuple(lhs.file_id, lhs.line, lhs.col) <
      std::make_tuple(rhs.file_id, rhs.line, rhs.col);
  };

  std::sort(references.begin(), references.end(), comp);

  auto begin = references.begin();
  auto end = begin;

  do
  {
    FileId current_file_id = begin->file_id;
    auto is_not_current_file = [&current_file_id](const SymbolReference& r) {
      return r.file_id != current_file_id;
    };
    end = std::find_if(begin, references.end(), is_not_current_file);

    size_t nb_refs = std::distance(begin, end);

    auto it = m_files_data.find(current_file_id);

    if (it == m_files_data.end())
    {
      m_files_data[current_file_id].num_references = nb_refs;
    }
    else
    {
      if (it->second.num_references != nb_refs)
      {
        // Mismatch, we need to compare to see the difference.
        // (this branch should be unlikely to happen)
        std::vector<SymbolReference> refsinfile = snapshot().listReferencesInFile(current_file_id);

        auto already_exists = [&refsinfile](const SymbolReference& r) {
          return std::find(refsinfile.begin(), refsinfile.end(), r) != refsinfile.end();
        };

        auto newend = std::remove_if(begin, end, already_exists);
        size_t remaining = std::distance(begin, newend);

        if (newend != end)
        {
          //std::cout << "erasing " << std::distance(newend, end) << " already existing references" << std::endl;
          end = references.erase(newend, end);
        }

        it->second.num_references += remaining;
      }
      else
      {
        // It's a match, we skip all the references
        end = references.erase(begin, end);
      }
    }

    begin = end;

  } while (begin != references.end());

}

} // namespace csnap
