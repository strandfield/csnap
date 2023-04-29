// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "version.h"

namespace csnap
{

namespace version
{

int major()
{
  return CSNAP_VERSION_MAJOR;
}

int minor()
{
  return CSNAP_VERSION_MINOR;
}

int patch()
{
  return CSNAP_VERSION_PATCH;
}

std::string suffix()
{
  return CSNAP_VERSION_SUFFIX;
}

} // namespace version

/**
 * \brief returns the csnap version as a string
 */
std::string versionstring()
{
  std::string r = std::to_string(version::major())
    + "." + std::to_string(version::minor())
    + "." + std::to_string(version::patch());

  if (!version::suffix().empty())
    r += "-" + version::suffix();

  return r;
}

} // namespace csnap
