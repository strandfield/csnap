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

/**
 * \brief writes a file on disk
 * \param p        the path on disk
 * \param content  the binary content of the file
 * 
 * If the file already exists, it is overwritten.
 * 
 * This function creates missing directories if \a p points 
 to a file in a directory that does not yet exist.
 */
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
