// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_TRANSLATIONUNIT_H
#define CSNAP_TRANSLATIONUNIT_H

#include "fileid.h"
#include "translationunitid.h"

#include <map>
#include <memory>
#include <set>
#include <string>

namespace csnap
{

namespace program
{

struct CompileOptions
{
  std::set<std::string> includedirs;
  std::map<std::string, std::string> defines;
};

} // namespace program

struct TranslationUnit
{
  TranslationUnitId id;
  FileId sourcefile_id;
  std::shared_ptr<const program::CompileOptions> compile_options;
};

} // namespace csnap

#endif // CSNAP_TRANSLATIONUNIT_H
