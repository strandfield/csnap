// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/snapshot.h"

#include "csnap/sql.h"

#include "csnap/file.h"
#include "csnap/include.h"
#include "csnap/symbols.h"

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

CREATE TABLE IF NOT EXISTS "base" (
  "symbol_id"              INTEGER,
  "num"                    INTEGER,
  "type"                   TEXT,
  "flags"                  INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id")
);

CREATE TABLE IF NOT EXISTS "parameter" (
  "id"                     INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "symbol_id"              INTEGER,
  "isTemplate"             INTEGER,
  "name"                   TEXT,
  "type"                   TEXT,
  "value"                  TEXT,
  "num"                    INTEGER,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id")
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
/*
void insert_into_db(csnap::Symbol& symbol, sql::Statement& q)
{
  bool ok = q.step() > 0;

  if (!ok)
  {
    std::cerr << "SQL query failed: " << q.toString() << std::endl;
    return;
  }

  symbol.dbid = sourceview::last_insert_row_id(db);
}
*/
void insert_fparam_into_database(Snapshot& snapshot, const Function& fn)
{
  sql::Statement stmt{ 
    snapshot.dbHandle(), 
    "INSERT INTO parameter (symbol_id, isTemplate, name, type, value, num) VALUES (?,?,?,?,?,?)"
  };

  for (size_t i(0); i < fn.parameters.size(); ++i)
  {
    FunctionParameter* fp = fn.parameters.at(i).get();

    stmt.bind(1, fp->id); 66 // how is it attributed?
    stmt.bind(2, false);
    stmt.bind(3, fp->name.c_str());
    stmt.bind(4, fp->type.c_str());

    if (!fp->default_value.empty())
      stmt.bind(5, fp->default_value.c_str());
    else
      stmt.bind(5, nullptr);

    stmt.bind(6, int(i));

    stmt.step();
  }

  stmt.finalize();
}

void insert_template_params_into_database(Snapshot& snapshot, int parent_id, const std::vector<std::unique_ptr<TemplateParameter>>& parameters)
{
  sql::Statement stmt{
    snapshot.dbHandle(),
    "INSERT INTO parameter (symbol_id, isTemplate, name, type, value, num) VALUES (?,?,?,?,?,?)"
  };

  stmt.bind(1, true);

  for (size_t i(0); i < parameters.size(); ++i)
  {
    TemplateParameter* tparam = parameters.at(i).get();

    stmt.bind(1, tparam->id);  66 // how is it attributed?
    stmt.bind(2, false);
    stmt.bind(3, tparam->name.c_str());

    if (tparam->isTypeParameter())
    {
      stmt.bind(4, "class");
      stmt.bind(5, tparam->defaultValue<TemplateTypeParameter>().c_str());
    }
    else
    {
      stmt.bind(4, tparam->nontypeParameter().type.c_str());
      stmt.bind(5, tparam->defaultValue<TemplateNonTypeParameter>().c_str());
    }

    stmt.bind(6, int(i));

    stmt.step();
  }
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

    if (c.is<ClassTemplate>())
      insert_template_params_into_database(snapshot, c.id, static_cast<const ClassTemplate&>(c).templateParameters());
  }
  break;
  case Whatsit::Function:
  case Whatsit::FunctionTemplate:
  {
    const auto& f = static_cast<const Function&>(sym);

    stmt.bind(6, f.return_type.c_str());

    stmt.step();

    insert_fparam_into_database(snapshot, f);

    if (f.is<csnap::FunctionTemplate>())
      insert_template_params_into_database(snapshot, f.id, static_cast<const FunctionTemplate&>(f).templateParameters());
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

} // namespace csnap
