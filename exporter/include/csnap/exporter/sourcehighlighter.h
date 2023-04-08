// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SOURCEHIGHLIGHTER_H
#define CSNAP_SOURCEHIGHLIGHTER_H

#include "definitiontable.h"
#include "filesema.h"
#include "iterator.h"
#include "pathresolver.h"

#include <csnap/model/filecontent.h>
#include <csnap/model/filelist.h>
#include <csnap/model/symbol.h>

#include <cpptok/tokenizer.h>

#include <filesystem>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

class XmlWriter;

namespace csnap
{

class SourceHighlighter
{
public:
  using SymbolPtr = std::shared_ptr<Symbol>;
  using SymbolMap = std::map<SymbolId, std::shared_ptr<Symbol>>;

  using TokenIterator = Iterator<std::vector<cpptok::Token>::const_iterator>;
  using IncludeIterator = Iterator<std::vector<Include>::const_iterator>;
  using ReferenceIterator = Iterator<std::vector<SymbolReference>::const_iterator>;

  struct SemaIterators
  {
    IncludeIterator include;
    ReferenceIterator reference;
  };

protected:
  PathResolver m_default_path_resolver;
  PathResolver* m_path_resolver;
  XmlWriter& xml;
  const FileContent& content;
  FileSema sema;
  const FileList& files;
  const SymbolMap& symbols;
  const DefinitionTable& definitions; // $TODO: make optional, only makes sense if we way to link to other pages
  std::filesystem::path m_file_path; // $TODO: group in a class with path resolver (e.g. SymbolLinks), make optional
  cpptok::Tokenizer lexer;
  int m_current_line = -1;
  std::unique_ptr<SemaIterators> current_line_sema;

public:

  SourceHighlighter(XmlWriter& xmlstream, const FileContent& fc, FileSema fm, const FileList& fs, const SymbolMap& ss, const DefinitionTable& defs);
  
  std::string rootPath() const;
  const std::filesystem::path& filePath() const;
  void setFilePath(const std::filesystem::path& fp);

  PathResolver& pathResolver() const;
  void setPathResolver(PathResolver& resolver);

  void writeLineSource(int l);

protected:

  int currentLine() const;
  const std::string_view& currentText() const;

  std::string pathHref(const std::filesystem::path& p) const;

  static std::string tagFromKeyword(const std::string& kw);

  static const std::string& cssClass(const Symbol& sym);

  static int getTokCol(std::string_view text, const cpptok::Token& tok);

  void insertPrecedingSpaces(std::string_view text, size_t& col, const cpptok::Token& tok);

  void writeToken(size_t& col, const cpptok::Token& tok);

  void writeToken(size_t& col, TokenIterator& tokit);

  static bool match(int line, const Include& incl);
  static bool match(size_t col, const SymbolReference& symref);

  virtual void writeTokenAnnotated(size_t& col, TokenIterator& tokit, IncludeIterator& inclit);
  virtual void writeTokenAnnotated(size_t& col, TokenIterator& tokit, ReferenceIterator& refit);

  void writeTokens(size_t& col, TokenIterator& tokit, SemaIterators& sema);

  void writeLineSource(const std::vector<cpptok::Token>& tokens, SemaIterators& sema);
};

} // namespace csnap

#endif // CSNAP_SOURCEHIGHLIGHTER_H

