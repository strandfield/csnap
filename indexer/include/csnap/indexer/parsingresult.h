// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_PARSINGRESULT_H
#define CSNAP_PARSINGRESULT_H

#include <csnap/model/translationunit.h>

#include <libclang-utils/clang-translation-unit.h>

#include <chrono>
#include <memory>

namespace csnap
{

struct TranslationUnitParsingResult
{
  TranslationUnit* source = nullptr;
  std::unique_ptr<libclang::TranslationUnit> result;
  std::chrono::milliseconds parsing_time;
};

} // namespace csnap

#endif // CSNAP_PARSINGRESULT_H
