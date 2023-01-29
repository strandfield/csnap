// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/snapshot.h"

#include "csnap/sql.h"

#include "csnap/file.h"
#include "csnap/include.h"
#include "csnap/symbols.h"
#include "csnap/use.h"

#include <cassert>
#include <filesystem>
#include <iostream>

namespace csnap
{

static const char* SQL_CREATE_STATEMENTS = R"(
BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS "info" (
  "id" INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "key" TEXT NOT NULL,
  "value" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "file" (
  "id"      INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "path"    TEXT NOT NULL,
  "content" TEXT
);

CREATE TABLE IF NOT EXISTS "include" (
  "file_id"                       INTEGER NOT NULL,
  "line"                          INTEGER NOT NULL,
  "included_file_id"              INTEGER NOT NULL,
  FOREIGN KEY("file_id")          REFERENCES "file"("id"),
  FOREIGN KEY("included_file_id") REFERENCES "file"("id")
);

CREATE TABLE IF NOT EXISTS "usr" (
  "id"   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "name" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "symwhatsit" (
  "id"   INTEGER NOT NULL PRIMARY KEY UNIQUE,
  "name" TEXT NOT NULL
);

INSERT INTO symwhatsit (id, name) VALUES(0, "class");
INSERT INTO symwhatsit (id, name) VALUES(1, "classtemplate");
INSERT INTO symwhatsit (id, name) VALUES(2, "enum");
INSERT INTO symwhatsit (id, name) VALUES(3, "enumvalue");
INSERT INTO symwhatsit (id, name) VALUES(4, "function");
INSERT INTO symwhatsit (id, name) VALUES(5, "functiontemplate");
INSERT INTO symwhatsit (id, name) VALUES(6, "functionparameter");
INSERT INTO symwhatsit (id, name) VALUES(7, "macro");
INSERT INTO symwhatsit (id, name) VALUES(8, "namespace");
INSERT INTO symwhatsit (id, name) VALUES(9, "templateparameter");
INSERT INTO symwhatsit (id, name) VALUES(10, "typedef");
INSERT INTO symwhatsit (id, name) VALUES(11, "typealias");
INSERT INTO symwhatsit (id, name) VALUES(12, "variable");

CREATE TABLE IF NOT EXISTS "symbol" (
  "id"                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "what"              INTEGER NOT NULL,
  "parent"            INTEGER,
  "name"              TEXT NOT NULL,
  "flags"             INTEGER NOT NULL DEFAULT 0,
  "type"              TEXT,
  "value"             TEXT,
  FOREIGN KEY("what") REFERENCES "symwhatsit"("id")
);

CREATE TABLE IF NOT EXISTS "definition" (
  "symbol_id"              INTEGER NOT NULL,
  "file_id"                INTEGER NOT NULL,
  "line"                   INTEGER,
  "col"                    INTEGER,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id"),
  FOREIGN KEY("file_id")   REFERENCES "file"("id")
);

CREATE TABLE IF NOT EXISTS "declaration" (
  "symbol_id"              INTEGER NOT NULL,
  "file_id"                INTEGER NOT NULL,
  "line"                   INTEGER,
  "col"                    INTEGER,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id"),
  FOREIGN KEY("file_id")   REFERENCES "file"("id")
);

CREATE TABLE IF NOT EXISTS "symbolreference" (
  "symbol_id"              INTEGER NOT NULL,
  "file_id"                INTEGER NOT NULL,
  "line"                   INTEGER,
  "col"                    INTEGER,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id"),
  FOREIGN KEY("file_id")   REFERENCES "file"("id")
);

COMMIT;
)";

Snapshot::~Snapshot()
{
  if (dbHandle())
    close();
}

sqlite3* Snapshot::dbHandle() const
{
  return m_database;
}

bool Snapshot::good() const
{
  return dbHandle() != nullptr;
}

bool Snapshot::open(const std::filesystem::path& dbPath)
{
  int r = sqlite3_open(dbPath.u8string().c_str(), &m_database);
  return r == SQLITE_OK;
}

void Snapshot::create(const std::filesystem::path& dbPath)
{
  if (std::filesystem::exists(dbPath))
    return;

  int r = sqlite3_open_v2(dbPath.u8string().c_str(), &m_database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

  assert(r == SQLITE_OK);

  char* error = nullptr;
  r = sqlite3_exec(dbHandle(), SQL_CREATE_STATEMENTS, NULL, NULL, &error);

  assert(r == SQLITE_OK);

  if (r != SQLITE_OK)
  {
    std::cerr << "SQLite error: " << error << std::endl;
    sqlite3_free(error);
  }
}

void Snapshot::close()
{
  if (!good())
    return;

  sqlite3_close(m_database);
  m_database = nullptr;
}

void set_snapshot_info(Snapshot& snapshot, const std::string& key, const std::string& value)
{
  sqlite3_stmt* stmt = nullptr;

  sqlite3_prepare_v2(snapshot.dbHandle(),
    "INSERT OR REPLACE INTO info (key, value) VALUES (?,?)",
    -1, &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, key.c_str(), -1, nullptr);
  sqlite3_bind_text(stmt, 2, value.c_str(), -1, nullptr);

  sqlite3_step(stmt);

  sqlite3_finalize(stmt);
}

std::string get_snapshot_info(const Snapshot& snapshot, const std::string& key)
{
  sqlite3_stmt* stmt = nullptr;

  sqlite3_prepare_v2(snapshot.dbHandle(),
    "SELECT value FROM info WHERE key = ?",
    -1, &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, key.c_str(), -1, nullptr);

  std::string result;

  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  }

  sqlite3_finalize(stmt);

  return result;
}

void insert_file(Snapshot& snapshot, const File& file)
{
  sql::Statement stmt{ snapshot.dbHandle(), "INSERT INTO file(id, path, content) VALUES(?,?,?)" };

  stmt.bind(1, file.id);
  stmt.bind(2, file.path.c_str());

  if (file.content.has_value())
    stmt.bind(3, file.content.value().c_str());
  else
    stmt.bind(3, nullptr);

  stmt.step();

  stmt.finalize();
}

void insert_includes(Snapshot& snapshot, const std::vector<Include>& includes)
{
  sql::Statement stmt{ snapshot.dbHandle(), "INSERT INTO include (file_id, line, included_file_id) VALUES(?,?,?)" };

  for (const Include& inc : includes)
  {
    stmt.bind(1, inc.file_id);
    stmt.bind(2, inc.line);
    stmt.bind(3, inc.included_file_id);

    stmt.step();
  }

  stmt.finalize();
}

static std::string join(const std::vector<FParam>& params)
{
  std::string r;

  for (const FParam& p : params)
  {
    r += p.toString() + ", ";
  }

  if (!params.empty())
  {
    r.pop_back();
    r.pop_back();
  }

  return r;
}

static std::string join(const std::vector<TParam>& tparams)
{
  std::string r;

  for (const TParam& p : tparams)
  {
    r += p.toString() + ", ";
  }

  if (!tparams.empty())
  {
    r.pop_back();
    r.pop_back();
  }

  return r;
}

static std::string typeField(const Function& f)
{
  std::string r;

  if (f.isTemplate())
    r += "<" + join(f.templateParameters()) + "> ";

  r += "(" + join(f.parameters) + ")";

  r += " -> " + f.return_type;

  return r;
}

static std::string join(const std::vector<BaseClass>& bases)
{
  std::string r;

  for (const BaseClass& b : bases)
  {
    r += b.toString() + ", ";
  }

  if (!bases.empty())
  {
    r.pop_back();
    r.pop_back();
  }

  return r;
}
static std::string typeField(const Class& c)
{
  std::string r;

  if (c.isTemplate())
    r += "<" + join(c.templateParameters()) + "> ";

  r += "[" + join(c.bases) + "]";

  return r;
}

void insert_symbol(Snapshot& snapshot, const Symbol& sym)
{
  sql::Statement stmt{ snapshot.dbHandle(), "INSERT INTO symbol(id, what, parent, name, flags, type, value) VALUES(?,?,?,?,?,?,?)" };

  stmt.bind(1, sym.id);
  stmt.bind(2, static_cast<int>(sym.whatsit()));

  if (sym.parent_id >= 0)
    stmt.bind(3, sym.parent_id);
  else
    stmt.bind(3, nullptr);

  stmt.bind(4, sym.name.c_str());

  stmt.bind(5, sym.flags);

  stmt.bind(6, nullptr);
  stmt.bind(7, nullptr);

  switch (sym.whatsit())
  {
  case Whatsit::Class:
  case Whatsit::ClassTemplate:
  {
    const auto& c = static_cast<const Class&>(sym);

    std::string type = typeField(c);
    stmt.bind(6, type.c_str());

    stmt.step();
  }
  break;
  case Whatsit::Function:
  case Whatsit::FunctionTemplate:
  {
    const auto& f = static_cast<const Function&>(sym);

    std::string type = typeField(f);
    stmt.bind(6, type.c_str());

    stmt.step();
  }
  break;
  case Whatsit::Namespace:
  case Whatsit::Enum:
  case Whatsit::EnumValue:
  {
    stmt.step();
  }
  break;
  case Whatsit::Variable:
  {
    const auto& var = static_cast<const Variable&>(sym);

    stmt.bind(6, var.type().c_str());
    stmt.bind(7, var.defaultValue().c_str());

    stmt.step();
  }
  break;
  case Whatsit::Typedef:
  {
    const auto& tpd = static_cast<const Typedef&>(sym);

    stmt.bind(6, tpd.typedef_type.c_str());

    stmt.step();
  }
  break;
  case Whatsit::TypeAlias:
  {
    const auto& talias = static_cast<const TypeAlias&>(sym);

    stmt.bind(6, talias.aliased_type.c_str());

    stmt.step();
  }
  break;
  default:
    break;
  }
}

void insert_symbol_uses(Snapshot& snapshot, const std::vector<SymbolUse>& uses)
{
  sql::Statement symref_query{ 
    snapshot.dbHandle(), 
    "INSERT INTO symbolreference (symbol_id, file_id, line, col) VALUES (?,?,?,?)" 
  };

  sql::Statement symdef_query{
    snapshot.dbHandle(),
    "INSERT INTO definition (symbol_id, file_id, line, col) VALUES (?,?,?,?)"
  };

  sql::Statement symdecl_query{
    snapshot.dbHandle(),
    "INSERT INTO declaration (symbol_id, file_id, line, col) VALUES (?,?,?,?)"
  };

  for (const SymbolUse& use : uses)
  {
    switch (use.howused)
    {
    case SymbolUse::Reference:
    {
      symref_query.bind(1, use.symbol_id);
      symref_query.bind(2, use.file_id);
      symref_query.bind(3, use.line);
      symref_query.bind(4, use.col);

      symref_query.step();
    }
    break;
    case SymbolUse::Declaration:
    {
      symdecl_query.bind(1, use.symbol_id);
      symdecl_query.bind(2, use.file_id);
      symdecl_query.bind(3, use.line);
      symdecl_query.bind(4, use.col);

      symdecl_query.step();
    }
    break;
    case SymbolUse::Definition:
    {
      symdef_query.bind(1, use.symbol_id);
      symdef_query.bind(2, use.file_id);
      symdef_query.bind(3, use.line);
      symdef_query.bind(4, use.col);

      symdef_query.step();
    }
    break;
    }
  }
}


} // namespace csnap
