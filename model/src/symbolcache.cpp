// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "symbolcache.h"

#include <stdexcept>
#include <vector>

namespace csnap
{

void SymbolCache::insert(std::shared_ptr<Symbol> s)
{
  m_data[s->id.value()] = s;
}

std::shared_ptr<Symbol> SymbolCache::find(SymbolId symid) const
{
  auto it = m_data.find(symid.value());

  if (it != m_data.end())
    return it->second.lock();
  else
    return nullptr;
}

void SymbolCache::clear()
{
  m_data.clear();
}

void SymbolCache::cleanup()
{
  for (auto it = m_data.begin(); it != m_data.end(); )
  {
    if (it->second.expired())
      it = m_data.erase(it);
    else
      ++it;
  }
}

} // namespace csnap
