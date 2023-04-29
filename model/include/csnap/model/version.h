// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_VERSION_H
#define CSNAP_VERSION_H

#include <string>

#define CSNAP_VERSION_MAJOR 0
#define CSNAP_VERSION_MINOR 1
#define CSNAP_VERSION_PATCH 0
#define CSNAP_VERSION_SUFFIX "dev"

namespace csnap
{

namespace version
{

int major();
int minor();
int patch();
std::string suffix();

} // namespace version

std::string versionstring();

} // namespace csnap

#endif // CSNAP_VERSION_H
