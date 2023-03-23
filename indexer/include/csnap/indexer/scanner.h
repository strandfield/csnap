// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SCANNER_H
#define CSNAP_SCANNER_H

#include "csnap/database/snapshot.h"

#include <filesystem>

namespace csnap
{

class Scanner
{
public:

  void initSnapshot(std::filesystem::path& p);

  void scanSln(const std::filesystem::path& slnPath);

private:
  std::unique_ptr<Snapshot> m_snapshot;
};

} // namespace csnap

#endif // CSNAP_SCANNER_H
