// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLLOADER_H
#define CSNAP_SYMBOLLOADER_H

#include "sql.h"

#include "csnap/model/symbol.h"

namespace csnap
{

class SymbolLoader
{
public:
  Database& database;
  Symbol symbol;

protected:
  sql::Statement m_query;

public:
  explicit SymbolLoader(Database& db);

  bool read(SymbolId id);
};

} // namespace csnap

#endif // CSNAP_SYMBOLLOADER_H
