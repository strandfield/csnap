// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "symbolloader.h"

namespace csnap
{

SymbolLoader::SymbolLoader(Database& db) :
  database(db),
  m_query(db, "SELECT id, what, parent, name, usr, displayname, flags FROM symbol WHERE id = ?")
{

}

bool SymbolLoader::read(SymbolId id)
{
  m_query.reset();

  m_query.bind(1, id.value());

  if (!m_query.step())
    return false;

  symbol.id = id;
  symbol.kind = static_cast<Whatsit>(m_query.columnInt(1));

  symbol.parent_id = m_query.nullColumn(2) ? SymbolId() : SymbolId(m_query.columnInt(2));
  symbol.name = m_query.column(3);
  symbol.usr = m_query.column(4);
  symbol.display_name = m_query.nullColumn(5) ? std::string() : m_query.column(5);
  symbol.flags = m_query.columnInt(6);

  return true;
}


SymbolEnumerator::SymbolEnumerator(Database& db) :
  database(db),
  m_query(db, "SELECT id, what, parent, name, usr, displayname, flags FROM symbol")
{

}

bool SymbolEnumerator::next()
{
  if (!m_query.step())
    return false;

  symbol.id = SymbolId(m_query.columnInt(0));
  symbol.kind = static_cast<Whatsit>(m_query.columnInt(1));

  symbol.parent_id = m_query.nullColumn(2) ? SymbolId() : SymbolId(m_query.columnInt(2));
  symbol.name = m_query.column(3);
  symbol.usr = m_query.column(4);
  symbol.display_name = m_query.nullColumn(5) ? std::string() : m_query.column(5);
  symbol.flags = m_query.columnInt(6);

  return true;
}

} // namespace csnap
