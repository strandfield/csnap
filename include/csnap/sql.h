// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SQL_H
#define CSNAP_SQL_H

#include <sqlite3.h>

#include <string>

namespace sql
{

class Statement
{
private:
  sqlite3* m_database = nullptr;
  sqlite3_stmt* m_statement = nullptr;

public:
  explicit Statement(sqlite3* db);
  Statement(sqlite3* db, const char* query);
  ~Statement();

  bool prepare(const char* query);
  bool step();
  void finalize();

  int rowid() const;

  void bind(int n, std::nullptr_t);
  void bind(int n, const char* text);
  void bind(int n, int value);

  std::string column(int n) const;
};

inline Statement::Statement(sqlite3* db)
  : m_database(db)
{

}

inline Statement::Statement(sqlite3* db, const char* query)
  : m_database(db)
{
  prepare(query);
}

inline Statement::~Statement()
{
  finalize();
}

inline bool Statement::prepare(const char* query)
{
  int r = sqlite3_prepare_v2(m_database, query, -1, &m_statement, nullptr);
  return r == SQLITE_OK;
}

inline bool Statement::step()
{
  int r = sqlite3_step(m_statement);
  return r == SQLITE_ROW;
}

inline void Statement::finalize()
{
  sqlite3_finalize(m_statement);
  m_statement = nullptr;
}

inline int Statement::rowid() const
{
  return sqlite3_last_insert_rowid(m_database);
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
  sqlite3_bind_int(m_statement, 1, value);
}

inline std::string Statement::column(int n) const
{
  return std::string(reinterpret_cast<const char*>(sqlite3_column_text(m_statement, n)));
}

} // namespace sql

#endif // CSNAP_SQL_H
