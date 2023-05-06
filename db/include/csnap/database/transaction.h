// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_TRANSACTION_H
#define CSNAP_TRANSACTION_H

#include "sql.h"

namespace sql
{

/**
 * \brief RAII class for sql transaction
 * 
 * This class can be used to automatically open and close a transaction 
 * in a C++ scope.
 */
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

/**
 * \brief opens a transaction on a database
 * \param db  the database
 */
inline Transaction::Transaction(Database& db) :
  m_database(db)
{
  sql::exec(m_database, "BEGIN TRANSACTION");
}

/**
 * \brief commits the transaction
 */
inline Transaction::~Transaction()
{
  sql::exec(m_database, "COMMIT");
}

} // namespace sql

#endif // CSNAP_TRANSACTION_H
