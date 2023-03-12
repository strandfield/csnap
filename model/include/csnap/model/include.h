// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_INCLUDE_H
#define CSNAP_INCLUDE_H

#include <optional>
#include <string>

namespace csnap
{

struct Include
{
  int file_id = -1;
  int line = -1;
  int included_file_id = -1;
};

} // namespace csnap

#endif // CSNAP_INCLUDE_H
