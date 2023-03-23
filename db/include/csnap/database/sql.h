// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SQL_H
#define CSNAP_SQL_H

#include "database.h"

#include <sqlite3.h>

#include <string>

namespace sql
{

using Database = csnap::Database;

class Statement
{
private:
  Database& m_database;
  sqlite3_stmt* m_statement = nullptr;

public:
  explicit Statement(Database& db);
  Statement(Database& db, const char* query);
  Statement(const Statement&) = delete;
  Statement(Statement&& other);
  ~Statement();

  bool prepare(const char* query);
  bool step();
  void reset();
  void finalize();

  std::string errormsg() const;
  int rowid() const;

  void bind(int n, std::nullptr_t);
  void bind(int n, const char* text);
  void bind(int n, int value);
  void bindBlob(int n, const std::string& bytes);

  std::string column(int n) const;
  int columnInt(int n) const;
};

inline Statement::Statement(Database& db)
  : m_database(db)
{

}

inline Statement::Statement(Database& db, const char* query)
  : m_database(db)
{
  prepare(query);
}

inline Statement::Statement(Statement&& other) :
  m_database(other.m_database),
  m_statement(other.m_statement)
{
  other.m_statement = nullptr;
}

inline Statement::~Statement()
{
  finalize();
}

inline bool Statement::prepare(const char* query)
{
  int r = sqlite3_prepare_v2(m_database.sqliteHandle(), query, -1, &m_statement, nullptr);
  return r == SQLITE_OK;
}

inline bool Statement::step()
{
  int r = sqlite3_step(m_statement);
  return r == SQLITE_ROW;
}

inline void Statement::reset()
{
  sqlite3_reset(m_statement);
}

inline void Statement::finalize()
{
  sqlite3_finalize(m_statement);
  m_statement = nullptr;
}

inline std::string Statement::errormsg() const
{
  return sqlite3_errmsg(m_database.sqliteHandle());
}

inline int Statement::rowid() const
{
  return (int)sqlite3_last_insert_rowid(m_database.sqliteHandle());
}

inline void Statement::bind(int n, std::nullptr_t)
{
  sqlite3_bind_null(m_statement, n);
}

inline void Statement::bind(int n, const char* text)
{
  sqlite3_bind_text(m_statement, n, text, -1, nullptr);
}

inline void Statement::bind(int n, int value)
{
  sqlite3_bind_int(m_statement, n, value);
}

inline void Statement::bindBlob(int n, const std::string& bytes)
{
  sqlite3_bind_blob(m_statement, n, bytes.c_str(), (int)bytes.size(), nullptr);
}

inline std::string Statement::column(int n) const
{
  return std::string(reinterpret_cast<const char*>(sqlite3_column_text(m_statement, n)));
}

inline int Statement::columnInt(int n) const
{
  return sqlite3_column_int(m_statement, n);
}

/*********************************************/

inline bool exec(Database& db, const std::string& query, std::string* error = nullptr)
{
  char* errorbuffer = nullptr;
  int r = sqlite3_exec(db.sqliteHandle(), query.c_str(), NULL, NULL, &errorbuffer);

  if (r != SQLITE_OK && error)
  {
    *error = errorbuffer;
    sqlite3_free(errorbuffer);
  }

  return r == SQLITE_OK;
}

} // namespace sql

#endif // CSNAP_SQL_H
