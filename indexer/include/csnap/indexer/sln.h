// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SLN_H
#define CSNAP_SLN_H

#include <filesystem>

namespace csnap
{

class Snapshot;

void openSln(const std::filesystem::path& path, Snapshot& snapshot);

} // namespace csnap

#endif // CSNAP_SLN_H
