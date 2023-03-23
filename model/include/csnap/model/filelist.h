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
  // @TODO: add a read-only flag somewhere so that the following members 
  // can be stored as vector<T> instead of vector<unique_ptr<T>>
  // Alternative: have a vector<T> that is filled when loading 
  // and a vector<unique_ptr<T>> that is used after loading, when 
  // new things are added; getFile(id) and other would have to check the  
  // vector<T> first, then the vector<unique_ptr<T>>.
  std::vector<std::unique_ptr<File>> m_files;

};

} // namespace csnap

#endif // CSNAP_FILELIST_H
