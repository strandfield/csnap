// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLLOADER_H
#define CSNAP_SYMBOLLOADER_H

#include "sql.h"

#include "csnap/model/symbol.h"

namespace csnap
{

class Snapshot;

/**
 * \brief helper class for loading symbols from the database
 */
class SymbolLoader
{
public:
  Database& database;

  /**
  * \brief the symbol that was last loaded
  * 
  * This symbol is only valid after a successful call to read().
  */
  Symbol symbol;

protected:
  sql::Statement m_query;

public:
  explicit SymbolLoader(Database& db);
  explicit SymbolLoader(const Snapshot& s);

  bool read(SymbolId id);
};

/**
 * \brief helper class for enumerating symbols in a snapshot
 */
class SymbolEnumerator
{
public:
  Database& database;

  /**
   * \brief the symbol that was last loaded
   * 
   * The symbol is only valid after a successful call to next().
   */
  Symbol symbol;

protected:
  sql::Statement m_query;

public:
  explicit SymbolEnumerator(Database& db);
  explicit SymbolEnumerator(const Snapshot& s);

  bool next();
};

} // namespace csnap

#endif // CSNAP_SYMBOLLOADER_H
