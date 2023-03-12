// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILE_H
#define CSNAP_FILE_H

#include <optional>
#include <string>

namespace csnap
{

struct File
{
  int id = -1;
  std::string path;
  std::optional<std::string> content;
};

} // namespace csnap

#endif // CSNAP_FILE_H
