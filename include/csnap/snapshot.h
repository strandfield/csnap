// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SNAPSHOT_H
#define CSNAP_SNAPSHOT_H

#include <filesystem>
#include <vector>

typedef struct sqlite3 sqlite3;

namespace csnap
{

class File;
class Include;
class Symbol;
struct SymbolUse;

class Snapshot
{
public:
  ~Snapshot();

  sqlite3* dbHandle() const;

  bool good() const;

  bool open(const std::filesystem::path& dbPath);

  void create(const std::filesystem::path& dbPath);

  void close();

private:
  sqlite3* m_database = nullptr;
};

void set_snapshot_info(Snapshot& snapshot, const std::string& key, const std::string& value);
std::string get_snapshot_info(const Snapshot& snapshot, const std::string& key);

void insert_file(Snapshot& snapshot, const File& file);
void insert_includes(Snapshot& snapshot, const std::vector<Include>& includes);
void insert_symbol(Snapshot& snapshot, const Symbol& sym);
void insert_symbol_uses(Snapshot& snapshot, const std::vector<SymbolUse>& uses);

} // namespace csnap

#endif // CSNAP_SNAPSHOT_H
