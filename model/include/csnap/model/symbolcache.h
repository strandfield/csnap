// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLCACHE_H
#define CSNAP_SYMBOLCACHE_H

#include "symbol.h"

#include <map>
#include <memory>

namespace csnap
{

class SymbolCache
{
public:

  void insert(std::shared_ptr<Symbol> s);

  template<typename It>
  void insert(It begin, It end);

  std::shared_ptr<Symbol> find(SymbolId symid) const;

  void clear();
  void cleanup();

private:
  std::map<int, std::weak_ptr<Symbol>> m_data;
};

template<typename It>
inline void SymbolCache::insert(It begin, It end)
{
  for (auto it = begin; it != end; ++it)
  {
    insert(*it);
  }
}

} // namespace csnap

#endif // CSNAP_SYMBOLCACHE_H
