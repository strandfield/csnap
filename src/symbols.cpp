// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/symbols.h"

#include <stdexcept>

namespace csnap
{

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

std::string whatsit2qstring(Whatsit w)
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
      return {};
  }
}

Whatsit Namespace::whatsit() const
{
  return ClassWhatsit;
}


Whatsit Class::whatsit() const
{
  return ClassWhatsit;
}

bool Class::isTemplate() const
{
  return false;
}

const std::vector<std::unique_ptr<TemplateParameter>>& Class::templateParameters() const
{
  static const std::vector<std::unique_ptr<TemplateParameter>> static_instance = {};
  return static_instance;
}

ClassTemplate::ClassTemplate(std::string name, Symbol* parent)
  : Class(std::move(name), parent)
{

}

ClassTemplate::ClassTemplate(std::vector<std::unique_ptr<TemplateParameter>> tparams, std::string name, Symbol* parent)
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

const std::vector<std::unique_ptr<TemplateParameter>>& ClassTemplate::templateParameters() const
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

Expression& Variable::defaultValue()
{
  return m_default_value;
}

const Expression& Variable::defaultValue() const
{
  return m_default_value;
}

int& Variable::specifiers()
{
  return m_flags;
}

int Variable::specifiers() const
{
  return m_flags;
}

bool Variable::isInline() const
{
  return specifiers() & VariableSpecifier::Inline;
}

bool Variable::isStatic() const
{
  return specifiers() & VariableSpecifier::Static;
}

bool Variable::isConstexpr() const
{
  return specifiers() & VariableSpecifier::Constexpr;
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

Whatsit Function::whatsit() const
{
  return ClassWhatsit;
}

bool Function::isTemplate() const
{
  return false;
}

const std::vector<std::unique_ptr<TemplateParameter>>& Function::templateParameters() const
{
  static const std::vector<std::unique_ptr<TemplateParameter>> static_instance = {};
  return static_instance;
}

static void write_params(const Function& f, std::string& out)
{
  for (const auto& p : f.parameters)
  {
    out += p->type;

    if (!p->name.empty())
      out += " " + p->name;

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

FunctionTemplate::FunctionTemplate(std::vector<std::unique_ptr<TemplateParameter>> tparams, std::string name, Symbol* parent)
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

const std::vector<std::unique_ptr<TemplateParameter>>& FunctionTemplate::templateParameters() const
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
