// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filepagelinker.h"

#include <cassert>

namespace csnap
{

FilePageLinker::FilePageLinker(PageURL url) : 
  m_url(std::move(url))
{

}

const PageURL& FilePageLinker::currentPageUrl() const
{
  return m_url;
}

void FilePageLinker::setCurrentPageUrl(PageURL url)
{
  m_url = std::move(url);
}

PathResolver* FilePageLinker::pathResolver() const
{
  return m_path_resolver;
}

void FilePageLinker::setPathResolver(PathResolver& resolver)
{
  m_path_resolver = &resolver;
}

std::string FilePageLinker::linkTo(const File& file) const
{
  assert(pathResolver());

  std::filesystem::path p = pathResolver()->filePath(file);
  return m_url.linkTo(p);
}

std::string FilePageLinker::linkTo(const File& file, int line) const
{
  return linkTo(file) + "#L" + std::to_string(line);
}

FilePageLinker::operator bool() const
{
  return m_path_resolver != nullptr;
}

} // namespace csnap
