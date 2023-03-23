// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filelist.h"

#include <algorithm>
#include <cassert>

namespace csnap
{

File* FileList::add(std::string path)
{
  m_files.push_back(std::make_unique<File>());
  m_files.back()->id = FileId((int)m_files.size() - 1);
  m_files.back()->path = std::move(path);
  return m_files.back().get();
}

File* FileList::add(std::unique_ptr<File> f)
{
  if(!f->id.valid())
    f->id = FileId((int)m_files.size());

  size_t i = (size_t)f->id.value();

  if (i < m_files.size())
  {
    assert(m_files.at(i) == nullptr);
  }

  if (m_files.size() <= i)
  {
    m_files.resize(i + 1);
  }

  m_files[i] = std::move(f);

  return m_files[i].get();
}

std::vector<File*> FileList::all() const
{
  std::vector<File*> r;
  r.reserve(m_files.size());

  std::for_each(m_files.begin(), m_files.end(), [&r](const std::unique_ptr<File>& f) {
    if (f) r.push_back(f.get());
    });

  return r;
}

File* FileList::get(Identifier<File> id) const
{
  if (!id.valid())
    return nullptr;

  size_t offset = (size_t)id.value();

  if (offset >= m_files.size())
    return nullptr;

  return m_files.at(offset).get();
}

File* FileList::find(const std::string& path) const
{
  auto it = std::find_if(m_files.begin(), m_files.end(), [&path](const FilePtr& f) {
    return f && f->path == path;
    });

  return it != m_files.end() ? it->get() : nullptr;
}

} // namespace csnap
