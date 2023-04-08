// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "sqlqueries.h"

#include "snapshot.h"
#include "sql.h"

#include "csnap/model/file.h"
#include "csnap/model/include.h"
#include "csnap/model/reference.h"
#include "csnap/model/symbol.h"
#include "csnap/model/translationunit.h"

#include <numeric>

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

CREATE TABLE "compileoptions" (
  "id"          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "defines"     TEXT NOT NULL,
  "includedirs" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "translationunit" (
  "id"                             INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
  "file_id"                        INTEGER NOT NULL,
  "compileoptions_id"              INTEGER,
  "ast"                            BLOB,
  FOREIGN KEY("file_id")           REFERENCES "file"("id"),
  FOREIGN KEY("compileoptions_id") REFERENCES "compileoptions"("id")
);

CREATE TABLE "ppinclude" (
  "translationunit_id"              INTEGER NOT NULL,
  "file_id"                         INTEGER NOT NULL,
  "line"                            INTEGER NOT NULL,
  "included_file_id"                INTEGER NOT NULL,
  FOREIGN KEY("translationunit_id") REFERENCES "translationunit"("id"),
  FOREIGN KEY("file_id")            REFERENCES "file"("id"),
  FOREIGN KEY("included_file_id")   REFERENCES "file"("id")
);

CREATE TABLE "include" (
  "file_id"                       INTEGER NOT NULL,
  "line"                          INTEGER NOT NULL,
  "included_file_id"              INTEGER NOT NULL,
  FOREIGN KEY("file_id")          REFERENCES "file"("id"),
  FOREIGN KEY("included_file_id") REFERENCES "file"("id"),
  UNIQUE(file_id, line)
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
  "usr"               TEXT NOT NULL,
  "displayname"       TEXT,
  "flags"             INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY("what") REFERENCES "symwhatsit"("id")
);

CREATE TABLE IF NOT EXISTS "symbolreference" (
  "symbol_id"                     INTEGER NOT NULL,
  "file_id"                       INTEGER NOT NULL,
  "line"                          INTEGER NOT NULL,
  "col"                           INTEGER NOT NULL,
  "parent_symbol_id"              INTEGER,
  "flags"                         INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY("symbol_id")        REFERENCES "symbol"("id"),
  FOREIGN KEY("file_id")          REFERENCES "file"("id"),
  FOREIGN KEY("parent_symbol_id") REFERENCES "symbol"("id")
);

CREATE TABLE IF NOT EXISTS "base" (
  "symbol_id"              INTEGER NOT NULL,
  "base_id"                INTEGER NOT NULL,
  "access_specifier"       INTEGER NOT NULL,
  FOREIGN KEY("symbol_id") REFERENCES "symbol"("id"),
  FOREIGN KEY("base_id")   REFERENCES "symbol"("id")

);

CREATE VIEW symboldefinition (symbol_id, file_id, line, col, flags) AS
  SELECT symbol_id, file_id, line, col, flags
  FROM symbolreference WHERE flags & 2;

COMMIT;
)";

template<typename T, typename F>
std::vector<T> read_vector(sql::Statement& stmt, F&& func)
{
  std::vector<T> r;

  while (stmt.step())
  {
    r.push_back(func(stmt));
  }

  return r;
}

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

std::vector<SymbolReference> select_from_symbolreference(Database& db, SymbolId symbol)
{
  sql::Statement stmt{ db, "SELECT file_id, line, col, parent_symbol_id, flags FROM symbolreference WHERE symbol_id = ?" };

  stmt.bind(1, symbol.value());

  return read_vector<SymbolReference>(stmt, [&symbol](sql::Statement& stmt) {
    SymbolReference symref;
    symref.file_id = stmt.columnInt(0);
    symref.symbol_id = symbol.value();
    symref.line = stmt.columnInt(1);
    symref.col = stmt.columnInt(2);

    if (stmt.nullColumn(3))
      symref.parent_symbol_id = SymbolId();
    else
      symref.parent_symbol_id = SymbolId(stmt.columnInt(3));

    symref.flags = stmt.columnInt(4);

    return symref;
    });
}

/**
 * \brief select all symbol references in a given file
 */
std::vector<SymbolReference> select_symbolreference(Database& db, FileId file)
{
  std::vector<SymbolReference> r;

  sql::Statement stmt{ db, "SELECT symbol_id, line, col, parent_symbol_id, flags FROM symbolreference WHERE file_id = ?" };

  stmt.bind(1, file.value());

  SymbolReference symref;
  symref.file_id = file.value();

  while (stmt.step())
  {
    symref.symbol_id = stmt.columnInt(0);
    symref.line = stmt.columnInt(1);
    symref.col = stmt.columnInt(2);

    if (stmt.nullColumn(3))
      symref.parent_symbol_id = SymbolId();
    else
      symref.parent_symbol_id = SymbolId(stmt.columnInt(3));

    symref.flags = stmt.columnInt(4);

    r.push_back(symref);
  }

  return r;
}

/**
 * \brief select all rows from the symboldefinition table
 * 
 * Note: currently the \em parent_symbol_id field of the SymbolReference 
 * struct is not filled.
 */
std::vector<SymbolReference> select_symboldefinition(Database& db)
{
  // $TODO: this could be rewritten using read_vector()

  std::vector<SymbolReference> r;

  sql::Statement stmt{ db, "SELECT symbol_id, file_id, line, col, flags FROM symboldefinition" };

  while (stmt.step())
  {
    SymbolReference symref;
    symref.symbol_id = stmt.columnInt(0);
    symref.file_id = stmt.columnInt(1);
    symref.line = stmt.columnInt(2);
    symref.col = stmt.columnInt(3);
    symref.flags = stmt.columnInt(4);

    /*
    if (stmt.nullColumn(4))
      symref.parent_symbol_id = SymbolId();
    else
      symref.parent_symbol_id = SymbolId(stmt.columnInt(3));
*/
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

void insert_file_content(Database& db, const std::vector<File*>& files)
{
  sql::Statement stmt{ db, "UPDATE file SET content = ? WHERE id = ?" };

  for (File* f : files)
  {
    if (!f)
      continue;

    std::filesystem::path filepath{ f->path };

    if (!std::filesystem::exists(filepath))
      continue;

    std::string bytes = Snapshot::readFile(filepath);

    stmt.bind(2, f->id.value());
    stmt.bind(1, bytes.c_str());

    stmt.step();
    stmt.reset();
  }

  stmt.finalize();
}

static std::string join(const std::vector<std::string>& list, char sep = ';')
{
  if (list.empty())
    return {};
  else if (list.size() == 1)
    return list.front();

  size_t s = std::accumulate(list.begin(), list.end(), size_t(0), [](size_t n, const std::string& str) {
    return n + str.size();
    });

  s += (list.size() - 1);

  std::string str;
  str.reserve(s);

  auto it = list.begin();

  for (; it != std::prev(list.end()); ++it)
  {
    str.insert(str.end(), it->begin(), it->end());
    str.push_back(sep);
  }

  str.insert(str.end(), it->begin(), it->end());

  return str;
}

static std::string join_defines(const std::map<std::string, std::string>& defines)
{
  std::vector<std::string> list;

  for (const std::pair<const std::string, std::string>& d : defines)
  {
    if (d.second.empty())
      list.push_back(d.first);
    else
      list.push_back(d.first + "=" + d.second);
  }

  return join(list);
}

static std::string join_includedirs(const std::set<std::string>& includedirs)
{
  return join(std::vector<std::string>(includedirs.begin(), includedirs.end()));
}

static int get_compile_options_id(std::map<const program::CompileOptions*, int>& map, sql::Statement& insert_query, const program::CompileOptions* copts)
{
  auto it = map.find(copts);

  if (it != map.end())
    return it->second;

  insert_query.bind(1, join_defines(copts->defines));
  insert_query.bind(2, join_includedirs(copts->includedirs));

  insert_query.step();
  insert_query.reset();

  int id = insert_query.rowid();

  map[copts] = id;

  return id;
}

void insert_translationunit(Database& db, const std::vector<TranslationUnit*>& units)
{
  std::map<const program::CompileOptions*, int> copts_ids;

  sql::Statement coptions_query{ db, "INSERT INTO compileoptions(defines, includedirs) VALUES(?,?)" };
  sql::Statement stmt{ db, "INSERT INTO translationunit(id, file_id, compileoptions_id) VALUES(?, ?, ?)" };

  for (TranslationUnit* tu : units)
  {
    stmt.bind(1, tu->id.value());
    stmt.bind(2, tu->sourcefile_id.value());
    stmt.bind(3, get_compile_options_id(copts_ids, coptions_query, tu->compile_options.get()));

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

void insert_ppinclude(Database& db, const TranslationUnit& tu, const std::vector<Include>& includes)
{
  sql::Statement stmt{ db, "INSERT INTO ppinclude (translationunit_id, file_id, line, included_file_id) VALUES(?,?,?,?)" };
  
  stmt.bind(1, tu.id.value());

  for (const Include& inc : includes)
  {
    stmt.bind(2, inc.file_id.value());
    stmt.bind(3, inc.line);
    stmt.bind(4, inc.included_file_id.value());

    stmt.step();
    stmt.reset();
  }

  stmt.finalize();
}

void insert_includes(Database& db, const std::vector<Include>& includes)
{
  // We use INSERT OR IGNORE here so that duplicates are automatically ignored by sqlite.
  // See the UNIQUE constraint in the CREATE statement of the "include" table.

  sql::Statement stmt{ db, "INSERT OR IGNORE INTO include (file_id, line, included_file_id) VALUES(?,?,?)" };

  for (const Include& inc : includes)
  {
    stmt.bind(1, inc.file_id.value());
    stmt.bind(2, inc.line);
    stmt.bind(3, inc.included_file_id.value());

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
  sql::Statement stmt{ db, "INSERT OR REPLACE INTO symbol(id, what, parent, name, usr, displayname, flags) VALUES(?,?,?,?,?,?,?)" };

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
    stmt.bind(5, sym.usr.c_str());

    if (!sym.display_name.empty())
      stmt.bind(6, sym.display_name.c_str());
    else
      stmt.bind(6, nullptr);

    stmt.bind(7, sym.flags);

    stmt.step();
    stmt.reset();
  }
}

void insert_symbol_references(Database& db, const std::vector<SymbolReference>& references)
{
  sql::Statement query{
    db,
    "INSERT INTO symbolreference (symbol_id, file_id, line, col, parent_symbol_id, flags) VALUES (?,?,?,?,?,?)"
  };

  for (const SymbolReference& ref : references)
  {
    query.bind(1, ref.symbol_id);
    query.bind(2, ref.file_id);
    query.bind(3, ref.line);
    query.bind(4, ref.col);

    if (ref.parent_symbol_id.valid())
      query.bind(5, ref.parent_symbol_id.value());
    else 
      query.bind(5, nullptr);

    query.bind(6, ref.flags);

    query.step();
    query.reset();
  }

  query.finalize();
}

void insert_base(Database& db, const std::map<SymbolId, std::vector<BaseClass>>& bases)
{
  sql::Statement query{
   db,
   "INSERT INTO base (symbol_id, base_id, access_specifier) VALUES (?,?,?)"
  };

  for (const std::pair<const SymbolId, std::vector<BaseClass>>& pair : bases)
  {
    query.bind(1, pair.first.value());

    for (const BaseClass& base : pair.second)
    {
      query.bind(2, base.base_id.value());
      query.bind(3, static_cast<int>(base.access_specifier));

      query.step();
      query.reset();
    }
  }

  query.finalize();
}

File read_file(sql::Statement& stmt, int id_col = 0, int path_col = 1)
{
  File f;
  f.id = FileId(stmt.columnInt(id_col));
  f.path = stmt.column(path_col);
  return f;
}

std::vector<File> select_file(Database& db)
{
  sql::Statement stmt{ db, "SELECT id, path FROM file" };
  return read_vector<File>(stmt, [](sql::Statement& q) {
    return read_file(q);
    });
}

std::string select_content_from_file(Database& db, FileId file)
{
  sql::Statement stmt{ db, "SELECT content FROM file WHERE id = ?" };
  stmt.bind(1, file.value());

  if (!stmt.step())
    return {};

  if (stmt.nullColumn(0))
    return {};
  else
    return stmt.column(0);
}

template<typename F>
static void split_apply(const std::string& liststr, char sep, F&& func)
{
  auto it = liststr.begin();
  auto next = std::find(liststr.begin(), liststr.end(), sep);

  while (next != liststr.end())
  {
    func(std::string(it, next));
    it = next + 1;
    next = std::find(it, liststr.end(), sep);
  }

  func(std::string(it, next));
}

static std::map<std::string, std::string> split_defines(const std::string& liststr)
{
  std::map<std::string, std::string> r;

  auto insert_define = [&r](std::string str) {
    if (str.empty())
      return;

    size_t n = str.find('=');

    if (n == std::string::npos)
    {
      r[str] = std::string();
    }
    else
    {
      r[str.substr(0, n)] = str.substr(n + 1);
    }
  };

  split_apply(liststr, ';', insert_define);

  return r;
}

static std::set<std::string> split_includes(const std::string& liststr)
{
  std::set<std::string> r;

  auto insert_if_not_empty = [&r](std::string str) {
    if (!str.empty())
      r.insert(std::move(str));
  };

  split_apply(liststr, ';', insert_if_not_empty);

  return r;
}

std::map<int, std::shared_ptr<program::CompileOptions>> select_compileoptions(Database& db)
{
  std::map<int, std::shared_ptr<program::CompileOptions>> dict;

  sql::Statement stmt{ db, "SELECT id, defines, includedirs FROM compileoptions" };

  while (stmt.step())
  {
    int id = stmt.columnInt(0);
    auto opt = std::make_shared<program::CompileOptions>();
    opt->defines = split_defines(stmt.column(1));
    opt->includedirs = split_includes(stmt.column(2));

    dict[id] = opt;
  }

  return dict;
}

std::vector<TranslationUnit> select_translationunit(Database& db)
{
  std::map<int, std::shared_ptr<program::CompileOptions>> copts = select_compileoptions(db);

  sql::Statement stmt{ db, "SELECT id, file_id, compileoptions_id FROM translationunit" };

  return read_vector<TranslationUnit>(stmt, [&copts](sql::Statement& stmt) {
    TranslationUnit tu;
    tu.id = TranslationUnitId(stmt.columnInt(0));
    tu.sourcefile_id = FileId(stmt.columnInt(1));
    tu.compile_options = copts[stmt.columnInt(2)];
    return tu;
    });
}

/**
 * \brief select rows from the include table
 * \param file_id           filter with respect to the file_id column
 * \param included_file_id  filter with respect to the included_file_id column
 * 
 * You can pass invalid file ids to this function, in which case no filtering will 
 * be applied to the column.
 * 
 * Example:
 * \code{.cpp}
 *   // Get all the files included by 'myfile':
 *   select_from_include(database, myfile, FileId());
 * // Get all the files that include by 'myfile':
 *   select_from_include(database, FileId(), myfile)
 * \endcode
 */
std::vector<Include> select_from_include(Database& db, FileId file_id, FileId included_file_id)
{
  std::map<int, std::shared_ptr<program::CompileOptions>> copts = select_compileoptions(db);

  sql::Statement stmt{ db, 
    "SELECT file_id, line, included_file_id FROM include "
    "WHERE (file_id = ?1 OR ?1 = -1) AND (included_file_id = ?2 OR ?2 = -1)"};

  stmt.bind(1, file_id.value());
  stmt.bind(2, included_file_id.value());

  return read_vector<Include>(stmt, [](sql::Statement& stmt) {
    Include r;
    r.file_id = FileId(stmt.columnInt(0));
    r.line = stmt.columnInt(1);
    r.included_file_id = FileId(stmt.columnInt(2));
    return r;
    });
}

} // namespace csnap
