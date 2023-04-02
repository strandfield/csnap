// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_WRITEFILE_H
#define CSNAP_WRITEFILE_H

#include <filesystem>
#include <fstream>
#include <string>

namespace csnap
{

inline void write_file(const std::filesystem::path& p, const std::string& content)
{
  std::filesystem::path dir = std::filesystem::absolute(p).parent_path();

  if (!std::filesystem::exists(dir))
    std::filesystem::create_directories(dir);

  std::ofstream file{ p, std::ios::binary | std::ios::trunc };
  file.write(content.data(), content.size());
}

} // namespace csnap

#endif // CSNAP_WRITEFILE_H
