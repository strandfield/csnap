// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filepagelinker.h"

#include <cassert>

namespace csnap
{

/**
 * \brief constructs a page linker
 * \param url  the url of the html page being written
 */
FilePageLinker::FilePageLinker(PageURL url) : 
  m_url(std::move(url))
{

}

/**
 * \brief returns the url of the current page
 * 
 * Thhis is the url passed to the constructor unless it was 
 * subsequently modified using setCurrentPageUrl().
 */
const PageURL& FilePageLinker::currentPageUrl() const
{
  return m_url;
}

void FilePageLinker::setCurrentPageUrl(PageURL url)
{
  m_url = std::move(url);
}

/**
 * \brief returns a pointer to the path resolver
 */
PathResolver* FilePageLinker::pathResolver() const
{
  return m_path_resolver;
}

/**
 * \brief sets the path resolver used to link to other pages
 */
void FilePageLinker::setPathResolver(PathResolver& resolver)
{
  m_path_resolver = &resolver;
}

/**
 * \brief computes a relative link to the html page for a file
 * \param file  the file to link to
 * 
 * A valid PathResolver must have been specified using setPathResolver() 
 * before calling this function.
 */
std::string FilePageLinker::linkTo(const File& file) const
{
  assert(pathResolver());

  std::filesystem::path p = pathResolver()->filePath(file);
  return m_url.linkTo(p);
}

/**
 * \brief computes a relative link to the html page for a file
 * \param file  the file to link to
 * \param line  the line to target
 * 
 * \overload std::string FilePageLinker::linkTo(const File& file) const
 */
std::string FilePageLinker::linkTo(const File& file, int line) const
{
  return linkTo(file) + "#L" + std::to_string(line);
}

/**
 * \brief returns whether this class is ready to produce links to other pages
 */
FilePageLinker::operator bool() const
{
  return m_path_resolver != nullptr;
}

} // namespace csnap
