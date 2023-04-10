// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filesema.h"

#include "csnap/model/symbolmap.h"

namespace csnap
{

inline static bool is_decl_or_def(const SymbolReference& r)
{
  return r.flags & (SymbolReference::Declaration | SymbolReference::Definition);
}

static bool swap_if_needed_and_discard_first(SymbolReference& first, SymbolReference& second, const std::map<SymbolId, std::shared_ptr<Symbol>>& symbols)
{
  std::shared_ptr<Symbol> first_symbol = find_symbol(symbols, SymbolId(first.symbol_id));
  std::shared_ptr<Symbol> second_symbol = find_symbol(symbols, SymbolId(second.symbol_id));

  if (!first_symbol || !second_symbol)
    return false;

  // The following if-elseif could probably be written in a more elegant way...
  if (first_symbol->kind == Whatsit::CXXConstructor && second_symbol->kind == Whatsit::CXXClass)
  {
    if (is_decl_or_def(first))
    {
      std::swap(first, second);
    }

    return true;
  }
  else if (first_symbol->kind == Whatsit::CXXClass && second_symbol->kind == Whatsit::CXXConstructor)
  {
    if (!is_decl_or_def(second))
    {
      std::swap(first, second);
    }

    return true;
  }

  return false;
}

/**
 * \brief simplify references to the class and its constructors at the same location
 * \param refs     the sorted vector of references within a file to process
 * \param symbols  the map of all symbols referenced in 'refs'
 * 
 * To declare a constructor, one cannot avoid referencing the class name.
 * This leads to both a reference to the class and to the constructor being located at 
 * the same position within a file.
 * 
 * There are other situations where both a class and one of its constructor are 
 * referenced at the same location (e.g., when the ctor is implicitely called with a 
 * member of the class).
 *   void f(MyClass a); f(MyClass::MyEnumValue);
 * 
 * When exporting a snapshot as HTML, one of the reference must be removed (depending on the context); 
 * which is what this function does.
 * When the constructor is declared or defined, the class reference must be removed; otherwise 
 * the constructor reference must be removed.
 */
void simplify_ctor_and_class_references(std::vector<SymbolReference>& refs, const std::map<SymbolId, std::shared_ptr<Symbol>>& symbols)
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
      if (swap_if_needed_and_discard_first(*it, *std::next(it), symbols))
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
