// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "writefile.h"

namespace csnap
{

void copy_resource(const std::string& name, const void* data, size_t nbbytes, const std::filesystem::path& outdir)
{
  auto content = std::string(reinterpret_cast<const char*>(data), nbbytes);
  write_file(outdir / name, content);
}

} // namespace csnap
