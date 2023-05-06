// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "pathresolver.h"

#include <algorithm>

namespace csnap
{

/**
 * \brief virtual destructor
 */
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

/**
 * \brief returns a url for a given file
 * \param f  a reference to the file
 * 
 * The default implementation simply returns the path of the file.
 * Subclasses should override this function to return a suitable path
 * (e.g., the url of an HTML page associated with \a f).
 */
std::filesystem::path PathResolver::filePath(const File& f) const
{
  return f.path;
}

} // namespace csnap
