// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_EXPRESSION_H
#define CSNAP_EXPRESSION_H

#include <string>

namespace csnap
{

using Expression = std::string;

inline bool is_null(const Expression& expr)
{
  return expr.empty();
}

} // namespace csnap

#endif // CSNAP_EXPRESSION_H
