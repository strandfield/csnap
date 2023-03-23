// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "sqlqueries.h"

#include "sql.h"

#include "csnap/model/file.h"
#include "csnap/model/include.h"
#include "csnap/model/reference.h"
#include "csnap/model/symbol.h"
#include "csnap/model/translationunit.h"

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

CREATE TABLE IF NOT EXISTS "translationunit" (
  "id"                   INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "file_id"              INTEGER NOT NULL,
  "ast"                  BLOB,
  FOREIGN KEY("file_id") REFERENCES "file"("id")
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

INSERT INTO symwhatsit (id, name) VALUES(0, "Unexposed");
INSERT INTO symwhatsit (id, name) VALUES(1, "Typedef");
INSERT INTO symwhatsit (id, name) VALUES(2, "Function");
INSERT INTO symwhatsit (id, name) VALUES(3, "Variable");
INSERT INTO symwhatsit (id, name) VALUES(4, "Field");
INSERT INTO symwhatsit (id, name) VALUES(5, "EnumConstant");
INSERT INTO symwhatsit (id, name) VALUES(6, "ObjCClass");
INSERT INTO symwhatsit (id, name) VALUES(7, "ObjCProtocol");
INSERT INTO symwhatsit (id, name) VALUES(8, "ObjCCategory");
INSERT INTO symwhatsit (id, name) VALUES(9, "ObjCInstanceMethod");
INSERT INTO symwhatsit (id, name) VALUES(10, "ObjCClassMethod");
INSERT INTO symwhatsit (id, name) VALUES(11, "ObjCProperty");
INSERT INTO symwhatsit (id, name) VALUES(12, "ObjCIvar");
INSERT INTO symwhatsit (id, name) VALUES(13, "Enum");
INSERT INTO symwhatsit (id, name) VALUES(14, "Struct");
INSERT INTO symwhatsit (id, name) VALUES(15, "Union");
INSERT INTO symwhatsit (id, name) VALUES(16, "CXXClass");
INSERT INTO symwhatsit (id, name) VALUES(17, "CXXNamespace");
INSERT INTO symwhatsit (id, name) VALUES(18, "CXXNamespaceAlias");
INSERT INTO symwhatsit (id, name) VALUES(19, "CXXStaticVariable");
INSERT INTO symwhatsit (id, name) VALUES(20, "CXXStaticMethod");
INSERT INTO symwhatsit (id, name) VALUES(21, "CXXInstanceMethod");
INSERT INTO symwhatsit (id, name) VALUES(22, "CXXConstructor");
INSERT INTO symwhatsit (id, name) VALUES(23, "CXXDestructor");
INSERT INTO symwhatsit (id, name) VALUES(24, "CXXConversionFunction");
INSERT INTO symwhatsit (id, name) VALUES(25, "CXXTypeAlias");
INSERT INTO symwhatsit (id, name) VALUES(26, "CXXInterface");

CREATE TABLE IF NOT EXISTS "symbol" (
  "id"                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "what"              INTEGER NOT NULL,
  "parent"            INTEGER,
  "name"              TEXT NOT NULL,
  "displayname"       TEXT,
  "flags"             INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY("what") REFERENCES "symwhatsit"("id")
);

CREATE TABLE IF NOT EXISTS "symbolreference" (
  "symbol_id"              INTEGER NOT NULL,
  "file_id"                INTEGER NOT NULL,
  "line"                   INTEGER NOT NULL,
  "col"                    INTEGER NOT NULL,
  "flags"                  INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id"),
  FOREIGN KEY("file_id")   REFERENCES "file"("id")
);

CREATE TABLE IF NOT EXISTS "base" (
  "symbol_id"              INTEGER NOT NULL,
  "base_id"                INTEGER NOT NULL,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id"),
  FOREIGN KEY("base_id")   REFERENCES "symbol"("id")

);

COMMIT;
)";

const char* db_init_statements()
{
  return SQL_CREATE_STATEMENTS;
}

void insert_info(Database& db, const std::string& key, const std::string& value)
{
  sqlite3_stmt* stmt = nullptr;

  sqlite3_prepare_v2(db.sqliteHandle(),
    "INSERT OR REPLACE INTO info (key, value) VALUES (?,?)",
    -1, &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, key.c_str(), -1, nullptr);
  sqlite3_bind_text(stmt, 2, value.c_str(), -1, nullptr);

  sqlite3_step(stmt);

  sqlite3_finalize(stmt);
}

std::string select_info(Database& db, const std::string& key)
{
  sqlite3_stmt* stmt = nullptr;

  sqlite3_prepare_v2(db.sqliteHandle(),
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

/**
 * \brief select all symbol references in a given file
 */
std::vector<SymbolReference> select_symbolreference(Database& db, FileId file)
{
  std::vector<SymbolReference> r;

  sql::Statement stmt{ db, "SELECT symbol_id, line, col, flags FROM symbolreference WHERE file_id = ?" };

  stmt.bind(1, file.value());

  SymbolReference symref;
  symref.file_id = file.value();

  while (stmt.step())
  {
    symref.symbol_id = stmt.columnInt(0);
    symref.line = stmt.columnInt(1);
    symref.col = stmt.columnInt(2);
    symref.flags = stmt.columnInt(3);

    r.push_back(symref);
  }

  return r;
}

void insert_file(Database& db, const File& file)
{
  sql::Statement stmt{ db, "INSERT INTO file(id, path) VALUES(?,?)" };

  stmt.bind(1, file.id.value());
  stmt.bind(2, file.path.c_str());

  stmt.step();

  stmt.finalize();
}

void insert_translationunit(Database& db, const std::vector<TranslationUnit*>& units)
{
  sql::Statement stmt{ db, "INSERT INTO translationunit(id, file_id) VALUES(?, ?)" };

  for (TranslationUnit* tu : units)
  {
    stmt.bind(1, tu->id.value());
    stmt.bind(2, tu->sourcefile_id.value());

    stmt.step();
    stmt.reset();
  }

  stmt.finalize();
}

void insert_translationunit_ast(Database& db, TranslationUnit* tu, const std::string& bytes)
{
  sql::Statement stmt{ db, "UPDATE translationunit SET ast = ? WHERE id = ?" };

  stmt.bind(2, tu->id.value());
  stmt.bindBlob(1, bytes);

  stmt.step();

  stmt.finalize();
}

void insert_includes(Database& db, const std::vector<Include>& includes)
{
  sql::Statement stmt{ db, "INSERT INTO include (file_id, line, included_file_id) VALUES(?,?,?)" };

  for (const Include& inc : includes)
  {
    stmt.bind(1, inc.file_id);
    stmt.bind(2, inc.line);
    stmt.bind(3, inc.included_file_id);

    stmt.step();
    stmt.reset();
  }

  stmt.finalize();
}

void insert_symbol(Database& db, const Symbol& sym)
{
  sql::Statement stmt{ db, "INSERT OR REPLACE INTO symbol(id, what, parent, name, displayname, flags) VALUES(?,?,?,?,?,?)" };

  stmt.bind(1, sym.id.value());
  stmt.bind(2, static_cast<int>(sym.kind));

  if (sym.parent_id.valid())
    stmt.bind(3, sym.parent_id.value());
  else
    stmt.bind(3, nullptr);

  stmt.bind(4, sym.name.c_str());

  if (!sym.display_name.empty())
    stmt.bind(5, sym.display_name.c_str());
  else
    stmt.bind(5, nullptr);

  stmt.bind(6, sym.flags);

  stmt.step();
}

void insert_symbol(Database& db, const std::vector<std::shared_ptr<Symbol>>& symbols)
{
  sql::Statement stmt{ db, "INSERT OR REPLACE INTO symbol(id, what, parent, name, displayname, flags) VALUES(?,?,?,?,?,?)" };

  for (auto sptr : symbols)
  {
    const Symbol& sym = *sptr;

    stmt.bind(1, sym.id.value());
    stmt.bind(2, static_cast<int>(sym.kind));

    if (sym.parent_id.valid())
      stmt.bind(3, sym.parent_id.value());
    else
      stmt.bind(3, nullptr);

    stmt.bind(4, sym.name.c_str());

    if (!sym.display_name.empty())
      stmt.bind(5, sym.display_name.c_str());
    else
      stmt.bind(5, nullptr);

    stmt.bind(6, sym.flags);

    stmt.step();
    stmt.reset();
  }
}

void insert_symbol_references(Database& db, const std::vector<SymbolReference>& references)
{
  sql::Statement query{
    db,
    "INSERT INTO symbolreference (symbol_id, file_id, line, col, flags) VALUES (?,?,?,?,?)"
  };

  for (const SymbolReference& ref : references)
  {
    query.bind(1, ref.symbol_id);
    query.bind(2, ref.file_id);
    query.bind(3, ref.line);
    query.bind(4, ref.col);
    query.bind(5, ref.flags);

    query.step();
    query.reset();
  }

  query.finalize();
}

void insert_bases(Database& db, const std::map<SymbolId, std::vector<BaseClass>>& bases)
{
  sql::Statement query{
   db,
   "INSERT INTO base (symbol_id, base_id) VALUES (?,?)"
  };

  for (const std::pair<const SymbolId, std::vector<BaseClass>>& pair : bases)
  {
    query.bind(1, pair.first.value());

    for (const BaseClass& base : pair.second)
    {
      query.bind(2, base.base_id.value());

      query.step();
      query.reset();
    }
  }

  query.finalize();
}

} // namespace csnap
