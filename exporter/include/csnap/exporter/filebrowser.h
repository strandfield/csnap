// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILEBROWSER_H
#define CSNAP_FILEBROWSER_H

#include "sourcehighlighter.h"

#include <set>

namespace csnap
{

class FileBrowserGenerator : public SourceHighlighter
{
public:

  FileBrowserGenerator(HtmlPage& p, const FileContent& fc, FileSema fm, const FileList& fs, const SymbolMap& ss, const DefinitionTable& defs);

  void generatePage();
  void generate();
  void generate(const std::set<int>& lines);

protected:

  void writeLine(size_t i);

  void writeCode();
};

} // namespace csnap

#endif // CSNAP_FILEBROWSER_H

