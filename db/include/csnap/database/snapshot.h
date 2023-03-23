// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SNAPSHOT_H
#define CSNAP_SNAPSHOT_H

#include "database.h"

#include "csnap/model/filelist.h"
#include "csnap/model/translationunitlist.h"
#include "csnap/model/reference.h"
#include "csnap/model/symbolcache.h"

#include <filesystem>
#include <memory>
#include <utility>

namespace csnap
{

struct PendingData;

class Snapshot
{
public:
  Snapshot() = delete; // @TODO: in-memory db ?
  Snapshot(const Snapshot&) = delete;
  Snapshot(Snapshot&&);
  ~Snapshot();

  explicit Snapshot(Database db);

  const Database& database() const;

  static Snapshot open(const std::filesystem::path& p);
  static Snapshot create(const std::filesystem::path& p);

  void setProperty(const std::string& key, const std::string& value);
  std::string property(const std::string& key) const;

  File* addFile(File f);
  void addFile(std::unique_ptr<File> f);
  void addFiles(const std::vector<File>& files);
  File* getFile(FileId id) const;
  File* findFile(const std::string& path) const;
  const FileList& files() const;

  void addTranslationUnits(const std::vector<FileId>& file_ids, program::CompileOptions opts);
  TranslationUnit* findTranslationUnit(File* file) const;
  TranslationUnit* getTranslationUnit(TranslationUnitId id) const;
  const TranslationUnitList& translationUnits() const;
  void addTranslationUnitSerializedAst(TranslationUnit* tu, const std::filesystem::path& astfile);

  void addSymbols(const std::vector<std::shared_ptr<Symbol>>& symbols);
  SymbolCache& symbolCache();

  void addSymbolReferences(const std::vector<SymbolReference>& list);
  std::vector<SymbolReference> listReferencesInFile(FileId file);

  bool hasPendingData() const;
  void writePendingData();

protected:
  PendingData& pendingData();
  void loadProgram();

private:
  std::unique_ptr<Database> m_database;
  FileList m_files; // $TODO: add a class that will generate ids for files, see also getOrCreateFile() in class Indexer
  TranslationUnitList m_translationunits;
  SymbolCache m_symbol_cache;
  std::unique_ptr<PendingData> m_pending_data;
};

} // namespace csnap

#endif // CSNAP_SNAPSHOT_H
