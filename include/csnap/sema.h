// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SEMA_H
#define CSNAP_SEMA_H

#include <memory>
#include <variant>
#include <vector>

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

using SemaVariant = std::variant<SymbolReference, SymbolDeclaration, SymbolDefinition>;
using SemaIterator = std::vector<SemaVariant>::const_iterator;

} // namespace csnap

#endif // CSNAP_CXX_SEMA_H
