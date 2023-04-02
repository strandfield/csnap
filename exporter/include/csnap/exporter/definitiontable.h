// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DEFINITIONTABLE_H
#define CSNAP_DEFINITIONTABLE_H

#include "csnap/model/symbolid.h"
#include "csnap/model/reference.h"

#include <vector>

namespace csnap
{

class DefinitionTable
{
public:

  void build(std::vector<SymbolReference> defs);
  
  bool hasUniqueDefinition(SymbolId id, SymbolReference* def = nullptr) const;

private:
  std::vector<SymbolReference> m_definitions;
  std::vector<size_t> m_table;
};

} // namespace cxx

#endif // CSNAP_DEFINITIONTABLE_H
