// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILECONTENTCACHE_H
#define CSNAP_FILECONTENTCACHE_H

#include "filecontent.h"

#include <map>
#include <memory>

namespace csnap
{

// $TODO: try refactor with SymbolCache
class FileContentCache
{
public:

  void insert(std::shared_ptr<FileContent> item);

  template<typename It>
  void insert(It begin, It end);

  std::shared_ptr<FileContent> find(FileId itemid) const;

  void clear();
  void cleanup();

private:
  std::map<int, std::weak_ptr<FileContent>> m_data;
};

template<typename It>
inline void FileContentCache::insert(It begin, It end)
{
  for (auto it = begin; it != end; ++it)
  {
    insert(*it);
  }
}

} // namespace csnap

#endif // CSNAP_FILECONTENTCACHE_H
