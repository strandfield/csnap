// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "pathresolver.h"

#include <algorithm>

namespace csnap
{

PathResolver::~PathResolver()
{

}

std::string PathResolver::relpath(const std::string& source, const std::string& target)
{
  if (target == source)
  {
    size_t n = target.rfind('/');

    if (n == std::string::npos)
      return target;
    else
      return target.substr(n + 1);
  }

  size_t minlen = std::min(source.size(), target.size());
  size_t i = 0;

  while (i < minlen && target.at(i) == source.at(i)) ++i;

  size_t nbdir = std::count(source.begin() + i, source.end(), '/');

  std::string r;
  r.reserve(target.size() - i + nbdir * 3);

  while (nbdir--)
  {
    r.append("../");
  }

  r.append(target.begin() + i, target.end());

  return r;
}


std::filesystem::path PathResolver::filePath(const File& f) const
{
  return f.path;
}

} // namespace csnap
