
# csnap

[![Build status](https://ci.appveyor.com/api/projects/status/find8sqtr41m8akm?svg=true)](https://ci.appveyor.com/project/strandfield/csnap)

`csnap` is a libclang-based command-line utility to create snapshots of C++ programs.

Snapshots produced by csnap are saved as a SQLite database, making them easily usable 
in other programs.
csnap can also export a snapshot as HTML so that it can be viewed online.

An HTML-export of csnap source code is browsable @ [strandfield.github.io/browse/csnap/index.html](https://strandfield.github.io/browse/csnap/index.html).

## Command-line interface (CLI)

**Creating a snapshot**

Syntax:
```
csnap scan --sln <Visual Studio Sln> --output <Database name> [--overwrite] [--threads <N>]
```

Description: 
Creates a snapshot of a C++ program from a Visual Studio solution.

Options:
- `--sln <Visual Studio Sln>`: specify the path of the Visual Studio solution `.sln` file (required)
- `--output <Database name>`: specify the path of SQLite database (required)
- `--overwrite`: specify that the output database should be overwritten if it already exists (optional)
- `--threads <N>`: specify the number of threads used for parsing the translation units (optional)

Examples: 

Create a snapshot using 4 threads for parsing:
```
csnap scan --sln build/csnap.sln --output snapshot.db --threads 4
```

**Exporting a snapshot as HTML**

Syntax:
```
csnap export --snapshot <Snapshot File> --output <Output directory>
```

Description: 
Exports a snapshot produced by `csnap scan` as HTML.

Options:
- `--snapshot <Snapshot File>`: specify the path of the snapshot (required)
- `--output <Output directory>`: specify the directory in which html files will be written (required)

Warning: csnap will overwrite files in the output directory.

Examples: 

Export a snapshot in `output/html`:
```
csnap export --snapshot snapshot.db --output output/html
```

## Continuous integration (CI)

**AppVeyor**

[![Build status](https://ci.appveyor.com/api/projects/status/find8sqtr41m8akm?svg=true)](https://ci.appveyor.com/project/strandfield/csnap)

The AppVeyor build is configured to take a snapshot of csnap and export 
it as HTML.
See `appveyor.yml` and the [build artifacts](https://ci.appveyor.com/project/strandfield/csnap/build/artifacts).

## Design & Architecture

### Overview

csnap uses [libclang](https://clang.llvm.org/doxygen/group__CINDEX.html) to parse and index the 
C++ translation units.

libclang is a C interface to Clang.
csnap most often does not use call libclang functions directly but rather 
uses [libclang-utils](https://github.com/strandfield/libclang-utils), a 
thin C++ wrapper for libclang.
Since libclang-utils loads the libclang (shared) library dynamically, csnap
does not need the clang headers to compile.

Internally, [clang_parseTranslationUnit2()](https://clang.llvm.org/doxygen/group__CINDEX__TRANSLATION__UNIT.html#ga494de0e725c5ae40cbdea5fa6081027d) 
is used to parse the translation units 
and [clang_indexTranslationUnit()](https://clang.llvm.org/doxygen/group__CINDEX__HIGH.html#gab12a0795c7d7be6e7ec85679faf3f8e9) 
is used for indexing.

Currently, the translation units are gathered from a Visual Studio Solution file (`.sln`).
An handmade parser, [vcxproj](https://github.com/strandfield/vcxproj), parses the solution 
and lists the translation units.

Snapshots are saved in a [SQLite](https://www.sqlite.org/index.html) database.
The database schema is partially reproduced below.

### Project structure

The project is divided into the following directories:

**model**

Contains the C++ `struct` that defines the various concepts used in a snapshot
(e.g., `File`, `TranslationUnit`, `Symbol`, `SymbolReference`, ...).

**db**

Contains the definition of the `Snapshot` class and other classes related 
to the database file.

**indexer**

This module is responsible for listing, parsing and indexing translation  
units to produce a snapshot.

**exporter**

This module provides the HTML export of a snapshot.

**src**

The source code that is used to produce the `csnap` executable
(mostly the command-line interface).
All other modules are static libraries used by this executable.

### Database Schema

Partial reproduction of the queries used to create the database
(see `sqlqueries.cpp` for more details).

```sql
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
```
