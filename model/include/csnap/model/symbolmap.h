// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLMAP_H
#define CSNAP_SYMBOLMAP_H

#include "symbol.h"

#include <map>

namespace csnap
{

/**
 * \brief finds a symbol in a map or return nullptr
 * \param symbols  the symbols map
 * \param id       the id of the symbol 
 */
inline std::shared_ptr<Symbol> find_symbol(const std::map<SymbolId, std::shared_ptr<Symbol>>& symbols, const SymbolId& id)
{
  auto it = symbols.find(id);
  return it != symbols.end() ? it->second : nullptr;
}

} // namespace csnap

#endif // CSNAP_SYMBOLMAP_H
