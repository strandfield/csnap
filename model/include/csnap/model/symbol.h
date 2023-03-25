// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOL_H
#define CSNAP_SYMBOL_H

#include "symbolid.h"

#include <string>

namespace csnap
{

enum class AccessSpecifier
{
  Invalid,
  Public,
  Protected,
  Private,
};

const std::string& to_string(AccessSpecifier aspec);

/**
 * \brief enum describing the kind of a symbol
 * 
 * The values are matching with libclang's CXIdxEntityKind enum.
 */
enum class Whatsit
{
  Unexposed = 0,
  Typedef = 1,
  Function = 2,
  Variable = 3,
  Field = 4,
  EnumConstant = 5,
  ObjCClass = 6,
  ObjCProtocol = 7,
  ObjCCategory = 8,
  ObjCInstanceMethod = 9,
  ObjCClassMethod = 10,
  ObjCProperty = 11,
  ObjCIvar = 12,
  Enum = 13,
  Struct = 14,
  Union = 15,
  CXXClass = 16,
  CXXNamespace = 17,
  CXXNamespaceAlias = 18,
  CXXStaticVariable = 19,
  CXXStaticMethod = 20,
  CXXInstanceMethod = 21,
  CXXConstructor = 22,
  CXXDestructor = 23,
  CXXConversionFunction = 24,
  CXXTypeAlias = 25,
  CXXInterface = 26
};

const std::string& whatsit2string(Whatsit w);

struct Symbol
{
  SymbolId id;
  Whatsit kind;
  std::string name;
  std::string usr;
  std::string display_name;
  SymbolId parent_id;
  int flags = 0;

  enum Flag
  {
    Local     = 0x0001,
    Public    = 0x0002,
    Protected = 0x0004,
    Private   = 0x0006,
    Inline    = 0x0008,
    Static    = 0x0010,
    Constexpr = 0x0020,
    IsScoped  = 0x0040,
    IsStruct  = 0x0040,
    IsFinal   = 0x0080,
    Virtual   = 0x0100,
    Override  = 0x0200,
    Final     = 0x0400,
    Const     = 0x0800,
    Pure      = 0x1000,
    Noexcept  = 0x2000,
    Explicit  = 0x4000,
    Default   = 0x8000,
    Delete    = 0x10000,
  };

public:
  explicit Symbol(Whatsit w, std::string name, Symbol* parent = nullptr);

};

inline bool is_local(const Symbol& s)
{
  return s.flags & Symbol::Local;
}

inline AccessSpecifier get_access_specifier(const Symbol& s)
{
  switch (s.flags & Symbol::Private)
  {
  case Symbol::Public:
    return AccessSpecifier::Public;
  case Symbol::Protected:
    return AccessSpecifier::Protected;
  case Symbol::Private:
    return AccessSpecifier::Private;
  default:
    return AccessSpecifier::Invalid;
  }
}

inline void set_access_specifier(Symbol& s, AccessSpecifier a)
{
  s.flags = s.flags & ~Symbol::Private;

  switch (a)
  {
  case AccessSpecifier::Public:
    s.flags |= Symbol::Public;
    break;
  case AccessSpecifier::Protected:
    s.flags |= Symbol::Protected;
    break;
  case AccessSpecifier::Private:
    s.flags |= Symbol::Private;
    break;
  }
}

struct BaseClass
{
  AccessSpecifier access_specifier = AccessSpecifier::Invalid;
  SymbolId base_id;

  std::string toString() const;
  static BaseClass fromString(std::string_view str);
};


inline bool is_public_base(const BaseClass& base)
{
  return base.access_specifier == AccessSpecifier::Public;
}

inline bool is_protected_base(const BaseClass& base)
{
  return base.access_specifier == AccessSpecifier::Protected;
}

inline bool is_private_base(const BaseClass& base)
{
  return base.access_specifier == AccessSpecifier::Private;
}

} // namespace csnap

#endif // CSNAP_SYMBOL_H
