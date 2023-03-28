// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "usrmap.h"

namespace csnap
{

SymbolId UsrMap::find(const std::string& usr) const
{
  auto it = m_map.find(usr);
  return it != m_map.end() ? it->second : SymbolId();
}

SymbolId UsrMap::insert(const std::string& usr)
{
  SymbolId new_id{ (int)m_map.size() };
  insert(usr, new_id);
  return new_id;
}

void UsrMap::insert(const std::string& usr, SymbolId id)
{
  m_map[usr] = id;
}

/**
 * \brief returns a copy of this usr map
 */
UsrMap GlobalUsrMap::clone()
{
  UsrMap copy;

  {
    std::lock_guard lock{ mutex() };
    copy = usrMap();
  }

  return copy;
}

std::pair<SymbolId, bool> GlobalUsrMap::get(const std::string& usr)
{
  std::lock_guard lock{ mutex() };

  SymbolId id = usrMap().find(usr);

  if (id.valid())
    return { id, false };

  return { usrMap().insert(usr), true };
}

UsrMap& GlobalUsrMap::usrMap()
{
  return m_map;
}

std::mutex& GlobalUsrMap::mutex()
{
  return m_mutex;
}

} // namespace csnap
