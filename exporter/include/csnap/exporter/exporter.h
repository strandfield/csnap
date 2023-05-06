// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_EXPORTER_H
#define CSNAP_EXPORTER_H

#include "csnap/database/snapshot.h"

#include <filesystem>
#include <map>

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

  /**
   * \brief root path for the input files
   * 
   * Within a snapshot, files have their complete filepath saved.
   * However, while exporting, it is often desirable to start at the level 
   * of the project's folder.
   * The \a rootpath variable can be used to specify that folder.
   * 
   * By default, this member contains a special character string that 
   * will make the exporter attempt to detect automatically a reasonable 
   * rootpath.
   */
  std::string rootpath;

public:
  explicit SnapshotExporter(Snapshot& s);

  void run();

protected:
  void detectRootPath();
  std::map<File*, std::filesystem::path> writeFilePages();
  void writeDirectoryPages(const std::map<File*, std::filesystem::path>& paths);
  void writeSymbolPages();
};

} // namespace csnap

#endif // CSNAP_EXPORTER_H

