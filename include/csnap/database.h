// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DATABASE_H
#define CSNAP_DATABASE_H

#if 0

#include "sourceview/database/database.h"

#include "sourceview/database/dto.h"
#include "sourceview/cxx/sema.h"
#include "sourceview/cxx/file-semantics.h"
#include "sourceview/snapshot/filemetadata.h"
#include "sourceview/vcs.h"

#include <memory>
#include <utility>
#include <vector>

namespace dto
{
struct File;
} // namespace dto

namespace csnap
{
class Symbol;
struct SymbolDefinition;
} // namespace csnap

namespace scan
{
class File;
} // namespace scan

struct Issue;
struct IssueDetection;
struct Rule;

namespace sourceview
{

void init_database(DatabaseSession& db);

void prepare_insert_file_statement(SqlStatement& stmt, dto::File& file);
void insert_file(DatabaseSession& db, const std::vector<std::shared_ptr<scan::File>>& files);
void set_file_sha1(DatabaseSession& db, const std::vector<std::shared_ptr<scan::File>>& files);

void insert_filemetrics(DatabaseSession& db, const std::vector<std::shared_ptr<scan::File>>& files);

void insert_rules_into_database(DatabaseSession& db, const std::vector<Rule>& rules);

void insert_issues_into_database(DatabaseSession& db, const std::vector<dto::Issue>& issues);

void insert_includes_into_database(DatabaseSession& db, const std::vector<std::shared_ptr<scan::File>>& files);

void insert_usr_into_database(DatabaseSession& db, const std::map<std::string, int>& usrs);
void insert_symbols_into_database(DatabaseSession& db, const std::vector<std::shared_ptr<csnap::Symbol>>& symbols);
void update_symbol_database(DatabaseSession& db, const std::vector<std::shared_ptr<csnap::Symbol>>& symbols);

void insert_sema_into_database(DatabaseSession& db, const std::vector<csnap::SemaVariant>& semalist);

} // namespace sourceview

#endif

#endif // CSNAP_DATABASE_H
