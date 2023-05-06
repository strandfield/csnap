// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_TRANSLATIONUNITID_H
#define CSNAP_TRANSLATIONUNITID_H

#include "identifier.h"

namespace csnap
{

struct TranslationUnit;

using TranslationUnitId = Identifier<TranslationUnit>;

} // namespace csnap

#endif // CSNAP_TRANSLATIONUNITID_H
