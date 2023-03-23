// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/model/symbol.h"

#include <stdexcept>
#include <vector>

namespace csnap
{

static std::string G_EMPTY_STR = "";

static std::string G_STR_PUBLIC = "public";
static std::string G_STR_PROTECTED = "protected";
static std::string G_STR_PRIVATE = "private";

const std::string& to_string(AccessSpecifier aspec)
{
  switch (aspec)
  {
  case AccessSpecifier::Public:
    return G_STR_PUBLIC;
  case AccessSpecifier::Protected:
    return G_STR_PROTECTED;
  case AccessSpecifier::Private:
    return G_STR_PRIVATE;
  default:
    return G_EMPTY_STR;
  }
}

const std::string& whatsit2string(Whatsit w)
{

  static const std::vector<std::string> strs = {
    "Unexposed",
    "Typedef",
    "Function",
    "Variable",
    "Field",
    "EnumConstant",
    "ObjCClass",
    "ObjCProtocol",
    "ObjCCategory",
    "ObjCInstanceMethod",
    "ObjCClassMethod",
    "ObjCProperty",
    "ObjCIvar",
    "Enum",
    "Struct",
    "Union",
    "CXXClass",
    "CXXNamespace",
    "CXXNamespaceAlias",
    "CXXStaticVariable",
    "CXXStaticMethod",
    "CXXInstanceMethod",
    "CXXConstructor",
    "CXXDestructor",
    "CXXConversionFunction",
    "CXXTypeAlias",
    "CXXInterface",
  };

  auto i = static_cast<size_t>(w);

  if (i >= strs.size())
    return G_EMPTY_STR;
  else
    return strs.at(i);
}

Symbol::Symbol(Whatsit w, std::string n, Symbol* parent) :
  kind(w),
  name(std::move(n)),
  parent_id(parent ? parent->id : SymbolId())
{

}

} // namespace csnap
