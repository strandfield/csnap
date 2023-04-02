// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filesema.h"

#include "csnap/model/symbolmap.h"

namespace csnap
{

static bool should_discard_first_of_pair(SymbolReference& first, SymbolReference& second, const std::map<SymbolId, std::shared_ptr<Symbol>>& symbols)
{
  std::shared_ptr<Symbol> first_symbol = find_symbol(symbols, SymbolId(first.symbol_id));
  std::shared_ptr<Symbol> second_symbol = find_symbol(symbols, SymbolId(second.symbol_id));

  if (!first_symbol || !second_symbol)
    return false;

  if (first_symbol->kind == Whatsit::CXXConstructor && second_symbol->kind == Whatsit::CXXClass)
    std::swap(first, second);

  return first_symbol->kind == Whatsit::CXXClass && second_symbol->kind == Whatsit::CXXConstructor;
}

/**
 * \brief remove references to the class associated with a constructor declaration or definition
 * \param refs     the sorted vector of references within a file to process
 * \param symbols  the map of all symbols referenced in 'refs'
 * 
 * To declare a constructor, one cannot avoid referencing the class name.
 * This leads to both a reference to the class and to the constructor being located at 
 * the same position within a file.
 * When exporting a snapshot as HTML, the reference to the class name must be removed; 
 * which is what this function does.
 */
void remove_constructors_class_reference(std::vector<SymbolReference>& refs, const std::map<SymbolId, std::shared_ptr<Symbol>>& symbols)
{
  auto read = refs.begin();
  auto write = read;

  while (read != refs.end())
  {
    auto it = std::adjacent_find(read, refs.end(), [](const SymbolReference& lhs, const SymbolReference& rhs) {
      return lhs.line == rhs.line && lhs.col == rhs.col;
      });

    if (it != refs.end())
    {
      if (should_discard_first_of_pair(*it, *std::next(it), symbols))
      {
        std::copy(read, it, write);
        write += std::distance(read, it);
        ++it;
      }
      else
      {
        ++it;
        std::copy(read, it, write);
        write += std::distance(read, it);
      }
    }
    else
    {
      std::copy(read, refs.end(), write);
      write += std::distance(read, it);
    }

    read = it;
  }

  if (write != refs.end())
  {
    refs.erase(write, refs.end());
  }
}

} // namespace csnap
