// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILELIST_H
#define CSNAP_FILELIST_H

#include "file.h"

#include <memory>
#include <vector>

namespace csnap
{

/**
 * \brief stores a list of files
 */
class FileList
{
public:

  File* add(std::string path);
  File* add(std::unique_ptr<File> f);
  std::vector<File*> all() const;
  File* get(Identifier<File> id) const;

  File* find(const std::string& path) const;

private:
  using FilePtr = std::unique_ptr<File>;
  // $TODO: consider another, more cache-friendly way to store the files ?
  std::vector<std::unique_ptr<File>> m_files;

};

} // namespace csnap

#endif // CSNAP_FILELIST_H
