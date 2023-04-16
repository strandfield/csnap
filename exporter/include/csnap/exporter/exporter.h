// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_EXPORTER_H
#define CSNAP_EXPORTER_H

#include "csnap/database/snapshot.h"

#include <filesystem>

namespace csnap
{

/**
 * \brief provides html export of a snapshot 
 */
class SnapshotExporter
{
public:
  /**
   * \brief reference to the snapshot passed at construction-time
   */
  Snapshot& snapshot;

  /**
   * \brief output directory in which the html files are going to be written
   * 
   * You may freely change this path before calling run().
   */
  std::filesystem::path outputdir;

public:
  explicit SnapshotExporter(Snapshot& s);

  void run();

protected:
  void writeSymbolPages();
};

} // namespace csnap

#endif // CSNAP_EXPORTER_H

