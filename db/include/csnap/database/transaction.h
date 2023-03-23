// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_TRANSACTION_H
#define CSNAP_TRANSACTION_H

#include "sql.h"

namespace sql
{

class Transaction
{
private:
  Database& m_database;

public:
  explicit Transaction(Database& db);
  Transaction(const Transaction&) = delete;
  Transaction(Transaction&&) = delete;
  ~Transaction();
};

inline Transaction::Transaction(Database& db) :
  m_database(db)
{
  sql::exec(m_database, "BEGIN TRANSACTION");
}

inline Transaction::~Transaction()
{
  sql::exec(m_database, "COMMIT");
}

} // namespace sql

#endif // CSNAP_TRANSACTION_H
