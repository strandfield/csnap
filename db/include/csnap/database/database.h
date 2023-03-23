// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DATABASE_H
#define CSNAP_DATABASE_H

#include <filesystem>

typedef struct sqlite3 sqlite3;

namespace csnap
{

class Database
{
public:
  Database() = default;
  Database(const Database&) = delete;
  Database(Database&& other);
  ~Database();

  sqlite3* sqliteHandle() const;

  bool good() const;

  bool open(const std::filesystem::path& dbPath);

  void create(const std::filesystem::path& dbPath);

  void close();

  Database& operator=(const Database&) = delete;
  Database& operator=(Database&& other);

private:
  sqlite3* m_database = nullptr;
};

} // namespace csnap

#endif // CSNAP_DATABASE_H
