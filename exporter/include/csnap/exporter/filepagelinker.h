// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILEPAGELINKER_H
#define CSNAP_FILEPAGELINKER_H

#include "pageurl.h"
#include "pathresolver.h"

#include <filesystem>

namespace csnap
{

class FilePageLinker
{
protected:
  PageURL m_url;
  PathResolver* m_path_resolver = nullptr;

public:
  explicit FilePageLinker(PageURL url);

  const PageURL& currentPageUrl() const;
  void setCurrentPageUrl(PageURL url);
 
  PathResolver* pathResolver() const;
  void setPathResolver(PathResolver& resolver);

  std::string linkTo(const File& file) const;
  std::string linkTo(const File& file, int line) const;

  operator bool() const;
};

} // namespace csnap

#endif // CSNAP_FILEPAGELINKER_H
