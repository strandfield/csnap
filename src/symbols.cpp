// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/symbols.h"

#include <stdexcept>

namespace csnap
{

static std::string G_EMPTY_STR = "";

static std::string G_STR_PUBLIC = "public";
static std::string G_STR_PROTECTED = "protected";
static std::string G_STR_PRIVATE = "private";

static std::string W_CLASS = "class";
static std::string W_CLASSTEMPLATE = "class-template";
static std::string W_ENUM = "enum";
static std::string W_ENUMVALUE = "enum-value";
static std::string W_FUNCTION = "function";
static std::string W_FUNCTIONTEMPLATE = "function-template";
static std::string W_FUNCTIONPARAMETER = "function-parameter";
static std::string W_MACRO = "macro";
static std::string W_NAMESPACE = "namespace";
static std::string W_TEMPLATEPARAMETER = "template-parameter";
static std::string W_TYPEDEF = "typedef";
static std::string W_TYPEALIAS = "type-alias";
static std::string W_VARIABLE = "variable";

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

  switch (w)
  {
    case Whatsit::Class:
      return W_CLASS;
    case Whatsit::ClassTemplate:
      return W_CLASSTEMPLATE;
    case Whatsit::Enum:
      return W_ENUM;
    case Whatsit::EnumValue:
      return W_ENUMVALUE;
    case Whatsit::Function:
      return W_ENUM;
    case Whatsit::FunctionTemplate:
      return W_FUNCTIONTEMPLATE;
    case Whatsit::FunctionParameter:
      return W_FUNCTIONPARAMETER;
    case Whatsit::Macro:
      return W_MACRO;
    case Whatsit::Namespace:
      return W_NAMESPACE;
    case Whatsit::TemplateParameter:
      return W_TEMPLATEPARAMETER;
    case Whatsit::Typedef:
      return W_TYPEDEF;
    case Whatsit::TypeAlias:
      return W_TYPEALIAS;
    case Whatsit::Variable:
      return W_VARIABLE;
    default:
      return G_EMPTY_STR;
  }
}

Whatsit Namespace::whatsit() const
{
  return ClassWhatsit;
}

std::string TParam::toString() const
{
  std::string r{};

  if (!name.empty())
    r += name + ": ";

  r += type;

  if (!default_value.empty())
    r += " = " + default_value;

  return r;
}

static bool is_identifier_char(char c)
{
  return (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= '0' && c <= '9') ||
    (c == '_');
}

static std::string read_param_name(std::string_view& str)
{
  size_t i = 0;

  while (i < str.size() && is_identifier_char(str[i])) ++i;

  if (i + 1 < str.size() && str[i] == ':' && str[i + 1] == ' ')
  {
    auto r = std::string(str.data(), str.data() + i);
    str = str.substr(i + 2);
    return r;
  }
  else
  {
    return {};
  }
}

TParam TParam::fromString(std::string_view str)
{
  TParam r;

  r.name = read_param_name(str);

  size_t valindex = str.find_last_of(" = ");

  if (valindex != std::string::npos)
  {
    r.default_value = std::string(str.substr(valindex + 3));
    str = str.substr(0, valindex);
  }

  r.type = std::string(str);

  return r;
}

std::string BaseClass::toString() const
{
  return to_string(access_specifier) + " " + base;
}

std::string_view read_token(std::string_view& str)
{
  size_t i = 0;

  while (i < str.size() && str[i] == ' ') ++i;

  size_t start = i;

  while (i < str.size() && str[i] != ' ') ++i;

  size_t end = i;

  auto r = std::string_view(str.data() + start, end - start);
  str = std::string_view(str.data() + end, str.size() - end);
  return r;
}

static bool starts_with(const std::string_view& text, const std::string_view& str)
{
  return text.size() >= str.size() && std::strncmp(text.data(), str.data(), str.size()) == 0;
}

BaseClass BaseClass::fromString(std::string_view str)
{
  BaseClass r;

  if (starts_with(str, "public "))
  {
    r.access_specifier = AccessSpecifier::Public;
    str = str.substr(7);
  }
  else if (starts_with(str, "protected "))
  {
    r.access_specifier = AccessSpecifier::Protected;
    str = str.substr(10);
  }
  else if (starts_with(str, "private "))
  {
    r.access_specifier = AccessSpecifier::Private;
    str = str.substr(8);
  }

  r.base = std::string(str);

  return r;
}

Whatsit Class::whatsit() const
{
  return ClassWhatsit;
}

bool Class::isTemplate() const
{
  return false;
}

const std::vector<TParam>& Class::templateParameters() const
{
  static const std::vector<TParam> static_instance = {};
  return static_instance;
}

ClassTemplate::ClassTemplate(std::string name, Symbol* parent)
  : Class(std::move(name), parent)
{

}

ClassTemplate::ClassTemplate(std::vector<TParam> tparams, std::string name, Symbol* parent)
  : Class(std::move(name), parent),
  template_parameters(std::move(tparams))
{
}

Whatsit ClassTemplate::whatsit() const
{
  return ClassTemplate::ClassWhatsit;
}

bool ClassTemplate::isTemplate() const
{
  return true;
}

const std::vector<TParam>& ClassTemplate::templateParameters() const
{
  return this->template_parameters;
}


EnumValue::EnumValue(std::string name, Enum* parent)
  : Symbol(std::move(name), parent)
{

}

EnumValue::EnumValue(std::string name, std::string value, Enum* parent)
  : Symbol(std::move(name), parent),
    m_value(std::move(value))
{

}

Whatsit EnumValue::whatsit() const
{
  return ClassWhatsit;
}

std::string& EnumValue::value()
{
  return m_value;
}

const std::string& EnumValue::value() const
{
  return m_value;
}


Whatsit Enum::whatsit() const
{
  return ClassWhatsit;
}



Typedef::Typedef(std::string name, Symbol* parent)
  : Symbol(std::move(name), parent)
{

}

Whatsit Typedef::whatsit() const
{
  return ClassWhatsit;
}



TypeAlias::TypeAlias(std::string name, Symbol* parent)
  : Symbol(std::move(name), parent)
{

}

Whatsit TypeAlias::whatsit() const
{
  return ClassWhatsit;
}


Whatsit Variable::whatsit() const
{
  return ClassWhatsit;
}

bool Variable::isInline() const
{
  return flags & Symbol::Inline;
}

bool Variable::isStatic() const
{
  return flags & Symbol::Static;
}

bool Variable::isConstexpr() const
{
  return flags & Symbol::Constexpr;
}



FunctionParameter::FunctionParameter(Type t, std::string name, Function* parent)
  : Symbol(std::move(name), parent),
    type(std::move(t))
{

}

FunctionParameter::FunctionParameter(Type t, std::string name, Expression default_val, Function* parent)
  : Symbol(std::move(name), parent),
    type(std::move(t)),
    default_value(default_val)
{

}

Whatsit FunctionParameter::whatsit() const
{
  return ClassWhatsit;
}

std::string FParam::toString() const
{
  std::string r{};

  if (!name.empty())
    r += name + ": ";

  r += type;

  if (!default_value.empty())
    r += " = " + default_value;

  return r;
}

FParam FParam::fromString(std::string_view str)
{
  FParam r;

  r.name = read_param_name(str);

  size_t valindex = str.find_last_of(" = ");

  if (valindex != std::string::npos)
  {
    r.default_value = std::string(str.substr(valindex + 3));
    str = str.substr(0, valindex);
  }

  r.type = std::string(str);

  return r;
}

Whatsit Function::whatsit() const
{
  return ClassWhatsit;
}

bool Function::isTemplate() const
{
  return false;
}

const std::vector<TParam>& Function::templateParameters() const
{
  static const std::vector<TParam> static_instance = {};
  return static_instance;
}

static void write_params(const Function& f, std::string& out)
{
  for (const auto& p : f.parameters)
  {
    out += p.type;

    if (!p.name.empty())
      out += " " + p.name;

    if (!p.default_value.empty())
      out += " = " + p.default_value;

    out += ", ";
  }

  if (!f.parameters.empty())
  {
    out.pop_back();
    out.pop_back();
  }
}

std::string Function::signature() const
{
  std::string result;
  if (isExplicit())
    result += "explicit ";
  if (isStatic())
    result += "static ";

  if (!this->return_type.empty())
  {
    result += this->return_type;
    result += " ";
  }

  result += this->name;
  result += "(";
  write_params(*this, result);
  result += ")";

  if (isConst())
    result += " const";

  if (isDelete())
    result += " = delete";

  return result;
}

FunctionTemplate::FunctionTemplate(std::string name, Symbol* parent)
  : Function(std::move(name), parent)
{

}

FunctionTemplate::FunctionTemplate(std::vector<TParam> tparams, std::string name, Symbol* parent)
  : Function(std::move(name), parent),
  template_parameters(std::move(tparams))
{
}

Whatsit FunctionTemplate::whatsit() const
{
  return FunctionTemplate::ClassWhatsit;
}

bool FunctionTemplate::isTemplate() const
{
  return true;
}

const std::vector<TParam>& FunctionTemplate::templateParameters() const
{
  return this->template_parameters;
}

Whatsit TemplateParameter::whatsit() const
{
  return ClassWhatsit;
}

TemplateParameter::TemplateParameter(TemplateTypeParameter ttp)
  : Symbol(std::move(name)),
  m_is_type_parameter(true)
{
  std::get<TemplateTypeParameter>(m_data) = ttp;
}

TemplateParameter::TemplateParameter(TemplateNonTypeParameter tntp)
  : Symbol(std::move(name)),
  m_is_type_parameter(false)
{
  std::get<TemplateNonTypeParameter>(m_data) = tntp;
}

bool TemplateParameter::isTypeParameter() const
{
  return m_is_type_parameter;
}

bool TemplateParameter::isNonTypeParameter() const
{
  return !m_is_type_parameter;
}

bool TemplateParameter::hasDefaultValue()
{
  return (isTypeParameter() && is_null(std::get<TemplateTypeParameter>(m_data).default_value))
    || (isNonTypeParameter() && is_null(std::get<TemplateNonTypeParameter>(m_data).default_value));
}

const TemplateNonTypeParameter& TemplateParameter::nontypeParameter() const
{
  return std::get<TemplateNonTypeParameter>(m_data);
}

} // namespace csnap
