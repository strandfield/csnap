// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/definition-table.h"

#include "csnap/symbols.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace csnap
{

void DefinitionTable::build(std::vector<csnap::SymbolDefinition> defs)
{
  m_definitions = std::move(defs);
  m_table.clear();

  if (m_definitions.empty())
    return;

  std::sort(m_definitions.begin(), m_definitions.end(),
    [](const SymbolDefinition& a, const SymbolDefinition& b) -> bool {
      return a.symbol_id < b.symbol_id;
    });

  int max_symbol_id = m_definitions.back().symbol_id;

  m_table.resize(static_cast<size_t>(max_symbol_id), std::numeric_limits<size_t>::max());

  int current_symbol_id = m_definitions.front().symbol_id;
  size_t current_start_offset = 0;

  for (size_t i(0); i < m_definitions.size(); ++i)
  {
    if (m_definitions.at(i).symbol_id != current_symbol_id)
    {
      auto symoffset = static_cast<size_t>(current_symbol_id);
      m_table[symoffset] = current_start_offset;

      current_symbol_id = m_definitions.at(i).symbol_id;
      current_start_offset = i;
    }
  }
}

bool DefinitionTable::hasUniqueDefinition(int symbolid, csnap::SymbolDefinition* def) const
{
  auto symoffset = static_cast<size_t>(symbolid);

  if (m_table.size() <= symoffset || m_table[symoffset] == std::numeric_limits<size_t>::max())
    return false;

  size_t firstdef = m_table[symoffset];

  if (firstdef + 1 == m_definitions.size() || m_definitions[firstdef + 1].symbol_id != symbolid)
  {
    if (def)
      *def = m_definitions[firstdef];

    return true;
  }
  else
  {
    return false;
  }
}

} // namespace csnap
