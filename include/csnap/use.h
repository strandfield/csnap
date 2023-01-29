// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_USE_H
#define CSNAP_USE_H

namespace csnap
{

struct SymbolReference
{
  int symbol_id;
  int file_id;
  int line;
  int col;
};

struct SymbolDeclaration
{
  int symbol_id;
  int file_id;
  int line;
  int col;
};

struct SymbolDefinition
{
  int symbol_id;
  int file_id;
  int line;
  int col;
};

struct SymbolUse
{
  enum How
  {
    Declaration,
    Definition,
    Reference,
  };

  int symbol_id;
  How howused;
  int file_id;
  int line;
  int col;

  SymbolUse(const SymbolReference& ref)
    : symbol_id(ref.symbol_id),
      howused(Reference),
      file_id(ref.file_id),
      line(ref.line),
      col(ref.col)
  {

  }

  SymbolUse(const SymbolDeclaration& decl)
    : symbol_id(decl.symbol_id),
      howused(Declaration),
      file_id(decl.file_id),
      line(decl.line),
      col(decl.col)
  {

  }

  SymbolUse(const SymbolDefinition& def)
    : symbol_id(def.symbol_id),
      howused(Definition),
      file_id(def.file_id),
      line(def.line),
      col(def.col)
  {

  }
};

} // namespace csnap

#endif // CSNAP_USE_H
