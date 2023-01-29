// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DEFINITION_TABLE_H
#define CSNAP_DEFINITION_TABLE_H

#include "use.h"

#include <vector>

namespace csnap
{

class DefinitionTable
{
public:

  void build(std::vector<SymbolDefinition> defs);
  
  bool hasUniqueDefinition(int symbolid, SymbolDefinition* def = nullptr) const;

private:
  std::vector<SymbolDefinition> m_definitions;
  std::vector<size_t> m_table;
};

} // namespace csnap

#endif // CSNAP_DEFINITION_TABLE_H
