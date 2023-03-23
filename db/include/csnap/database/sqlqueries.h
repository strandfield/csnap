// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SQLQUERIES_H
#define CSNAP_SQLQUERIES_H

#include "database.h"

#include "csnap/model/fileid.h"
#include "csnap/model/symbolid.h"

#include <map>
#include <string>
#include <vector>

namespace csnap
{

struct BaseClass;
struct File;
struct Include;
struct SymbolReference;
struct Symbol;
struct TranslationUnit;

const char* db_init_statements();

void insert_info(Database& db, const std::string& key, const std::string& value);
std::string select_info(Database& db, const std::string& key);

std::vector<SymbolReference> select_symbolreference(Database& db, FileId file);

void insert_file(Database& db, const File& file);
void insert_translationunit(Database& db, const std::vector<TranslationUnit*>& units);
void insert_translationunit_ast(Database& db, TranslationUnit* tu, const std::string& bytes);
void insert_ppinclude(Database& db, const TranslationUnit& tu, const std::vector<Include>& includes);
void insert_includes(Database& db, const std::vector<Include>& includes);
void insert_symbol(Database& db, const Symbol& sym);
void insert_symbol(Database& db, const std::vector<std::shared_ptr<Symbol>>& symbols);
void insert_symbol_references(Database& db, const std::vector<SymbolReference>& references);
void insert_base(Database& db, const std::map<SymbolId, std::vector<BaseClass>>& bases);

} // namespace csnap

#endif // CSNAP_SQLQUERIES_H
