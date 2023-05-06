// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLPAGE_H
#define CSNAP_SYMBOLPAGE_H

#include "htmlpage.h"
#include "pathresolver.h"

#include "csnap/database/snapshot.h"

namespace csnap
{

class SymbolPageGenerator
{
public:
  HtmlPage& page;
  Snapshot& snapshot;
  const Symbol& symbol;

public:

  SymbolPageGenerator(HtmlPage& p, Snapshot& snap, const Symbol& sym);

  PathResolver* pathResolver() const;
  void setPathResolver(PathResolver& resolver);

  void writePage();

protected:
  void writeBody();
  void writeSummary();
  void writeBases();
  void writeDerivedClasses();
  void writeDecls(const std::vector<SymbolReference>& list);
  void writeUses(const std::vector<SymbolReference>& list);
  using RefIterator = std::vector<SymbolReference>::const_iterator;
  void writeUsesInFile(RefIterator begin, RefIterator end);
};

} // namespace csnap

#endif // CSNAP_SYMBOLPAGE_H

