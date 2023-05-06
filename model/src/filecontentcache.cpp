// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filecontentcache.h"

#include <stdexcept>
#include <vector>

namespace csnap
{

void FileContentCache::insert(std::shared_ptr<FileContent> item)
{
  m_data[item->file->id.value()] = item;
}

std::shared_ptr<FileContent> FileContentCache::find(FileId itemid) const
{
  auto it = m_data.find(itemid.value());

  if (it != m_data.end())
    return it->second.lock();
  else
    return nullptr;
}

void FileContentCache::clear()
{
  m_data.clear();
}

void FileContentCache::cleanup()
{
  for (auto it = m_data.begin(); it != m_data.end(); )
  {
    if (it->second.expired())
      it = m_data.erase(it);
    else
      ++it;
  }
}

} // namespace csnap
