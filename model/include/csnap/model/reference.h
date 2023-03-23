// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_REFERENCE_H
#define CSNAP_REFERENCE_H

namespace csnap
{

/**
 * \brief stores information about a reference to a symbol
 */
struct SymbolReference
{
  int symbol_id;
  int file_id;
  int line;
  int col;
  int flags = 0;

  /**
   * \brief flag values for a symbol reference
   * 
   * This enum mirrors libclang's CXSymbolRole.
   */
  enum Flag
  {
    Declaration = 1 << 0,
    Definition = 1 << 1,
    Reference = 1 << 2,
    Read = 1 << 3,
    Write = 1 << 4,
    Call = 1 << 5,
    Dynamic = 1 << 6,
    AddressOf = 1 << 7,
    Implicit = 1 << 8
  };
};

inline bool operator==(const SymbolReference& lhs, const SymbolReference& rhs)
{
  return lhs.symbol_id == rhs.symbol_id
    && lhs.file_id == rhs.file_id
    && lhs.line == rhs.line
    && lhs.col == rhs.col
    && lhs.flags == rhs.flags;
}

inline bool operator!=(const SymbolReference& lhs, const SymbolReference& rhs)
{
  return !(lhs == rhs);
}

} // namespace csnap

#endif // CSNAP_REFERENCE_H
