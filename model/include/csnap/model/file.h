// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILE_H
#define CSNAP_FILE_H

#include "fileid.h"

#include <string>

namespace csnap
{

struct File
{
  FileId id;
  std::string path;
};

inline File create_file(std::string path, FileId id = {})
{
  File f;
  f.id = id;
  f.path = std::move(path);
  return f;
}

} // namespace csnap

#endif // CSNAP_FILE_H
