// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_EXPORTER_H
#define CSNAP_EXPORTER_H

#include "csnap/database/snapshot.h"

#include <filesystem>

namespace csnap
{

class SnapshotExporter
{
public:
  Snapshot& snapshot;
  std::filesystem::path outputdir;

public:

  explicit SnapshotExporter(Snapshot& s);

  void run();

};

} // namespace csnap

#endif // CSNAP_EXPORTER_H

