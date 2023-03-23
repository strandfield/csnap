// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLID_H
#define CSNAP_SYMBOLID_H

#include "identifier.h"

namespace csnap
{

struct Symbol;

using SymbolId = Identifier<Symbol>;

} // namespace csnap

#endif // CSNAP_SYMBOLID_H
