// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_SYMBOLS_H
#define CSNAP_SYMBOLS_H

#include "expression.h"
#include "type.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <vector>

namespace csnap
{

class Class;
class Enum;
class EnumValue;
class Function;
class Namespace;
class TemplateParameter;
class Variable;

enum class AccessSpecifier
{
  Invalid,
  Public,
  Protected,
  Private,
};

const std::string& to_string(AccessSpecifier aspec);

enum class Whatsit
{
  Class = 0,
  ClassTemplate,
  Enum,
  EnumValue,
  Function,
  FunctionTemplate,
  FunctionParameter,
  Macro,
  Namespace,
  TemplateParameter,
  Typedef,
  TypeAlias,
  Variable,
};

const std::string& whatsit2string(Whatsit w);

struct LocalInfo
{
  int id = -1;
  // @TODO: remove line & col
  int line = -1;
  int col = -1;
};

class Symbol
{
public:
  int id = -1;
  std::string name;
  int parent_id = -1;
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
  explicit Symbol(std::string name, Symbol* parent = nullptr);

  virtual Whatsit whatsit() const = 0;
  Whatsit kind() const { return whatsit(); }

  template<typename T>
  bool is() const;

  bool isLocal() const;
};

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

template<typename T, typename U = std::shared_ptr<Symbol>>
std::shared_ptr<T> find(const std::vector<U>& symbols, const std::string& name)
{
  auto it = std::find_if(symbols.begin(), symbols.end(), [&name](const std::shared_ptr<Symbol>& e) {
    return e->is<T>() && e->name == name;
    });

  if (it != symbols.end())
    return std::static_pointer_cast<T>(*it);
  else
    return nullptr;
}

class Namespace : public Symbol
{
public:
  ~Namespace() = default;

  static constexpr Whatsit ClassWhatsit = Whatsit::Namespace;
  Whatsit whatsit() const override;

  explicit Namespace(std::string name, Symbol* parent = nullptr);
};

struct TParam
{
  std::string type;
  std::string name;
  std::string default_value;

  std::string toString() const;
  static TParam fromString(std::string_view str);
};

struct BaseClass
{
  AccessSpecifier access_specifier = AccessSpecifier::Invalid;
  Type base;

  bool isPublicBase() const
  {
    return access_specifier == AccessSpecifier::Public;
  }

  bool isProtectedBase() const
  {
    return access_specifier == AccessSpecifier::Protected;
  }

  bool isPrivateBase() const
  {
    return access_specifier == AccessSpecifier::Private;
  }

  std::string toString() const;
  static BaseClass fromString(std::string_view str);
};

class Class : public Symbol
{
public:
  std::vector<BaseClass> bases;

public:
  ~Class() = default;

  explicit Class(std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::Class;
  Whatsit whatsit() const override;

  virtual bool isTemplate() const;
  virtual const std::vector<TParam>& templateParameters() const;
};

class ClassTemplate : public Class
{
public:
  std::vector<TParam> template_parameters;

public:
  explicit ClassTemplate(std::string name, Symbol* parent = nullptr);
  ClassTemplate(std::vector<TParam> tparams, std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::ClassTemplate;
  Whatsit whatsit() const override;

  bool isTemplate() const override;
  const std::vector<TParam>& templateParameters() const override;
};


class EnumValue : public Symbol
{
public:
  explicit EnumValue(std::string name, Enum* parent = nullptr);
  EnumValue(std::string name, std::string value, Enum* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::EnumValue;
  Whatsit whatsit() const override;

  std::string& value();
  const std::string& value() const;

private:
  std::string m_value;
};


class Enum : public Symbol
{
public:
  std::vector<std::shared_ptr<EnumValue>> values;

public:
  ~Enum() = default;

  explicit Enum(std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::Enum;
  Whatsit whatsit() const override;

  bool isScoped() const;
};

class Typedef : public Symbol
{
public:
  Type typedef_type;

public:
  ~Typedef() = default;

  explicit Typedef(std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::Typedef;
  Whatsit whatsit() const override;
};


class TypeAlias : public Symbol
{
public:
  csnap::Type aliased_type;

public:
  ~TypeAlias() = default;

  explicit TypeAlias(std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::TypeAlias;
  Whatsit whatsit() const override;
};

class Variable : public Symbol
{
public:
  Variable(Type type, std::string name, Symbol* parent = nullptr);
  Variable(Type type, std::string name, Expression default_value, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::Variable;
  Whatsit whatsit() const override;

  Type& type();
  const Type& type() const;

  Expression& defaultValue();
  const Expression& defaultValue() const;

  bool isInline() const;
  bool isStatic() const;
  bool isConstexpr() const;

public:
  Type m_type;
  Expression m_default_value;
};


class FunctionParameter : public Symbol
{
public:
  Type type;
  Expression default_value;

public:
  FunctionParameter(Type type, std::string name, Function* parent = nullptr);
  FunctionParameter(Type type, std::string name, Expression default_value, Function* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::FunctionParameter;
  Whatsit whatsit() const override;
};

struct FParam
{
  Type type;
  std::string name;
  Expression default_value;

  std::string toString() const;
  static FParam fromString(std::string_view str);
};

class Function : public Symbol
{
public:
  Type return_type;
  std::vector<FParam> parameters;

public:
  ~Function() = default;

  explicit Function(std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::Function;
  Whatsit whatsit() const override;

  typedef FunctionParameter Parameter;

  virtual bool isTemplate() const;
  virtual const std::vector<TParam>& templateParameters() const;

  bool isInline() const;
  bool isStatic() const;
  bool isConstexpr() const;
  bool isVirtual() const;
  bool isVirtualPure() const;
  bool isOverride() const;
  bool isFinal() const;
  bool isConst() const;
  bool isNoexcept() const;
  bool isExplicit() const;
  bool isDefault() const;
  bool isDelete() const;

  std::string signature() const;
};

class FunctionTemplate : public Function
{
public:
  std::vector<TParam> template_parameters;

public:
  explicit FunctionTemplate(std::string name, Symbol* parent = nullptr);
  FunctionTemplate(std::vector<TParam> tparams, std::string name, Symbol* parent = nullptr);

  static constexpr Whatsit ClassWhatsit = Whatsit::FunctionTemplate;
  Whatsit whatsit() const override;

  bool isTemplate() const override;
  const std::vector<TParam>& templateParameters() const override;
};


struct TemplateNonTypeParameter
{
  Type type;
  Expression default_value;

  typedef Expression default_value_t;
};

struct TemplateTypeParameter
{
  Type default_value;

  typedef Type default_value_t;
};

class TemplateParameter : public Symbol
{
private:
  bool m_is_type_parameter;
  std::tuple<TemplateNonTypeParameter, TemplateTypeParameter> m_data;

public:
  TemplateParameter() = delete;
  TemplateParameter(const TemplateParameter&) = default;
  TemplateParameter(TemplateParameter&&) = default;
  ~TemplateParameter() = default;

  TemplateParameter(TemplateTypeParameter ttp);
  TemplateParameter(TemplateNonTypeParameter tntp);

  static constexpr Whatsit ClassWhatsit = Whatsit::TemplateParameter;
  Whatsit whatsit() const override;

  bool isTypeParameter() const;
  bool isNonTypeParameter() const;

  bool hasDefaultValue();

  const TemplateNonTypeParameter& nontypeParameter() const;

  template<typename T>
  typename T::default_value_t defaultValue() const
  {
    return std::get<T>(m_data).default_value;
  }

  TemplateParameter& operator=(const TemplateParameter&) = default;
  TemplateParameter& operator=(TemplateParameter&&) = default;
};

} // namespace csnap

namespace csnap
{

template<typename T>
bool test_node_kind(const Symbol& n)
{
  return T::ClassWhatsit == n.kind();
}

template<>
inline bool test_node_kind<Class>(const Symbol& n)
{
  return n.kind() == Whatsit::Class || n.kind() == Whatsit::ClassTemplate;
}

template<>
inline bool test_node_kind<Function>(const Symbol& n)
{
  return n.kind() == Whatsit::Function || n.kind() == Whatsit::FunctionTemplate;
}

inline Symbol::Symbol(std::string n, Symbol* parent)
  : name(std::move(n)),
    parent_id(parent ? parent->id : -1)
{

}

template<typename T>
inline bool Symbol::is() const
{
  return test_node_kind<T>(*this);
}

inline bool Symbol::isLocal() const
{
  return (flags & Local);
}

inline Namespace::Namespace(std::string name, Symbol* parent)
  : Symbol{ std::move(name), parent }
{

}

inline Class::Class(std::string name, Symbol* parent)
  : Symbol{ std::move(name), parent }
{

}

inline Enum::Enum(std::string name, Symbol* parent)
  : Symbol{ std::move(name), parent }
{

}

inline bool Enum::isScoped() const
{
  return (flags & Symbol::IsScoped);
}

inline Variable::Variable(Type type, std::string name, Symbol* parent)
  : Symbol{ std::move(name), parent },
  m_type{ type }
{

}

inline Variable::Variable(Type type, std::string name, Expression default_value, Symbol* parent)
  : Symbol{ std::move(name), parent },
  m_type{ type },
  m_default_value{ std::move(default_value) }
{

}

inline Type& Variable::type()
{
  return m_type;
}

inline const Type& Variable::type() const
{
  return m_type;
}


inline Function::Function(std::string name, Symbol* parent)
  : Symbol{ std::move(name), parent }
{

}

inline bool Function::isInline() const
{
  return (flags & Symbol::Inline);
}

inline bool Function::isStatic() const
{
  return (flags & Symbol::Static);
}

inline bool Function::isConstexpr() const
{
  return (flags & Symbol::Constexpr);
}

inline bool Function::isVirtual() const
{
  return (flags & Symbol::Virtual);
}

inline bool Function::isVirtualPure() const
{
  return (flags & Symbol::Pure);
}

inline bool Function::isOverride() const
{
  return (flags & Symbol::Override);
}

inline bool Function::isFinal() const
{
  return (flags & Symbol::Final);
}

inline bool Function::isConst() const
{
  return (flags & Symbol::Const);
}

inline bool Function::isNoexcept() const
{
  return (flags & Symbol::Noexcept);
}

inline bool Function::isExplicit() const
{
  return (flags & Symbol::Explicit);
}

inline bool Function::isDefault() const
{
  return (flags & Symbol::Default);
}

inline bool Function::isDelete() const
{
  return (flags & Symbol::Delete);
}

} // namespace csnap

#endif // CSNAP_SYMBOLS_H
