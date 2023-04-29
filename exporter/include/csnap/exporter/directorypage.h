// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DIRECTORYPAGE_H
#define CSNAP_DIRECTORYPAGE_H

#include "htmlpage.h"

#include <map>

namespace csnap
{

class DirectoryPageGenerator
{
public:
  HtmlPage& page;
  const std::map<File*, std::filesystem::path>& all_pages;
  std::filesystem::path working_dir;
  
public:

  DirectoryPageGenerator(HtmlPage& p, const std::map<File*, std::filesystem::path>& allpages, std::filesystem::path wd);

  void writePage();

protected:
  void writeBody();
};

} // namespace csnap

#endif // CSNAP_DIRECTORYPAGE_H

