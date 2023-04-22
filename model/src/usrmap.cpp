// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "usrmap.h"

namespace csnap
{

/**
 * \brief returns the id associated with a usr
 * \param usr
 * 
 * If the \a usr cannot be found in the map, this returns an invalid SymbolId.
 */
SymbolId UsrMap::find(const std::string& usr) const
{
  auto it = m_map.find(usr);
  return it != m_map.end() ? it->second : SymbolId();
}

/**
 * \brief inserts a new usr into the map
 * \param usr
 * 
 * This function assigns an unused id to \a usr and returns the id.
 */
SymbolId UsrMap::insert(const std::string& usr)
{
  SymbolId new_id{ (int)m_map.size() };
  insert(usr, new_id);
  return new_id;
}

/**
 * \brief inserts a new usr into the map
 * \param usr  the usr
 * \param id   the id to assign to the usr
 * 
 * Note that this function currently does not verify whether \a usr is already 
 * in the map or if \a id is already associated with another usr.
 */
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

/**
 * \brief get a symbol id from a usr
 * \param usr  the usr
 * 
 * This function returns a pair with the symbol id and a boolean indicating 
 * whether the usr was inserted into the map (true if inserted; false if already present).
 */
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
