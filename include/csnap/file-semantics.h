// Copyright (C) 2022 Vincent Chambrin
// This file is part of the 'sourceview' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef SOURCEVIEW_CXX_FILESEMANTICS_H
#define SOURCEVIEW_CXX_FILESEMANTICS_H

#include <memory>
#include <variant>
#include <vector>

namespace csnap
{

class Symbol;

class FileInclude
{
public:
  int lineno = -1;
  int fileid = -1;

public:
  FileInclude() = default;
  FileInclude(int l, int fid);

  int line() const;
  int file() const;
};

// @TODO: move to analyzer ?

// @TODO: since the weak_ptr may be replaced by ids, 
// we may need a struct to distinguish between a local symbol and a global symbol ?
// The problem is when we load the file data, if weak_ptr, this means we must load the 
// symbols when a file uses it.
// Is this a problem? If a file uses a symbol should it be loaded right away or should 
// we wait.
// If the file is loaded, it is likely the user will ask information about the symbols 
// in the file so loading the symbol right away doesn't seem to be a bad strategy.
// Eager loading seems a good strategy for now.
// But later, when we do unloading, this could be problematic.
// If we unload a file that is referenced by a FileInclude, the weak_ptr will lose 
// the data and the id won't be recoverable (unless we reload the whole file).
// So long term strategy the id should probably be stored,
// but short term the weak_ptr will do.
struct SemaSymbolReference
{
  int line = -1;
  int col = -1;
  std::weak_ptr<csnap::Symbol> symbol; // @TODO: replace by id ?

  int get_line() const { return line; }
  int get_col() const { return col; }
};

struct SemaSymbolDefinition
{
  int line = -1;
  int col = -1;
  std::weak_ptr<csnap::Symbol> symbol; // @TODO: replace by id?

  int get_line() const { return line; }
  int get_col() const { return col; }
};

struct SemaSymbolDeclaration
{
  int line = -1;
  int col = -1;
  std::weak_ptr<csnap::Symbol> symbol; // @TODO: replace by id?

  int get_line() const { return line; }
  int get_col() const { return col; }
};

class SemaFileInclude
{
public:
  int lineno;
  int col;
  int fileid;

public:
  SemaFileInclude(int l, int c, int f)
    : lineno(l), col(c), fileid(f) { }

  int line() const { return lineno; }
  int file() const { return fileid; }

  int get_line() const { return lineno; }
  int get_col() const { return col; }
};

using FileSemaVariant = std::variant<SemaSymbolReference, SemaSymbolDefinition, SemaSymbolDeclaration, SemaFileInclude>;
using FileSemaIterator = std::vector<FileSemaVariant>::const_iterator;

// @TODO: separate into multiple classes ?
// User may be interested just in the list of included files
// Or included files + (sema & symbols)
// @TODO: or maybe merge with the coverage information into a FileMetaData class,
// the would be a typedef FileMetaData MetaData in the File class.
// see code below
class FileSemantics
{
public:
  std::vector<FileInclude> includes;

  std::vector<FileSemaVariant> sema;
  std::vector<std::shared_ptr<csnap::Symbol>> localsymbols; // @TODO: store symbol + def location

  void addInclude(int file_id, int line, int col);
};

csnap::FileSemantics& get_filesemantics(std::shared_ptr<FileSemantics>& ptr);

int get_line(const FileSemaVariant& sema);
int get_col(const FileSemaVariant& sema);
int comp(const FileSemaVariant& lhs, const FileSemaVariant& rhs);

} // namespace csnap

#endif // SOURCEVIEW_CXX_FILESEMANTICS_H
