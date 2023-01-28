// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DEFINITION_TABLE_H
#define CSNAP_DEFINITION_TABLE_H

#include "sema.h"

#include <vector>

namespace csnap
{

class Symbol;

class DefinitionTable
{
public:

  void build(std::vector<csnap::SymbolDefinition> defs);
  
  bool hasUniqueDefinition(int symbolid, csnap::SymbolDefinition* def = nullptr) const;

private:
  std::vector<csnap::SymbolDefinition> m_definitions;
  std::vector<size_t> m_table;
};

} // namespace csnap

#endif // CSNAP_DEFINITION_TABLE_H
