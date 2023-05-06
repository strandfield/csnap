// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_PATHRESOLVER_H
#define CSNAP_PATHRESOLVER_H

#include <csnap/model/file.h>

#include <filesystem>

namespace csnap
{

/**
 * \brief helper class that produces page url for files saved in a snapshot
 */
class PathResolver
{
public:
  virtual ~PathResolver();

  static std::string relpath(const std::string& source, const std::string& target);

  virtual std::filesystem::path filePath(const File& f) const;
};

} // namespace csnap

#endif // CSNAP_PATHRESOLVER_H

