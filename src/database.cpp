// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/database.h"

#if 0

#include "scanner/scan.h"

#include "sourceview/database/dto.h"

#include "sourceview/cxx/entities.h"
#include "sourceview/cxx/file-semantics.h"

#include "sourceview/issues.h"
#include "sourceview/rule.h"

#include <Poco/Data/Session.h>

namespace sourceview
{

void insert_usr_into_database(DatabaseSession& db, const std::map<std::string, int>& usrs)
{
  int id;
  std::string usr;
  SqlStatement query{ db };

  query << "INSERT INTO usr (sid, name) "
    "VALUES (?,?)",
    db::use(id),
    db::use(usr);

  for (const auto& p : usrs)
  {
    id = p.second;
    usr = p.first;

    query.execute();
  }
}

void insert_into_db(csnap::Symbol& symbol, DatabaseSession& db, SqlStatement& q)
{
  bool ok = q.execute() > 0;

  if (!ok)
  {
    std::cerr << "SQL query failed: " << q.toString() << std::endl;
    return;
  }

  symbol.dbid = sourceview::last_insert_row_id(db);
}

void insert_fparam_into_database(DatabaseSession& db, const csnap::Function& fn)
{
  dto::FunctionParameter data;

  SqlStatement query{ db };

  query << "INSERT INTO parameter (symbol_sid, isTemplate, name, type, value, num) "
    "VALUES (?,?,?,?,?,?)",
    db::use(data.symbol_id),
    db::use(data.is_template),
    db::use(data.name),
    db::use(data.type),
    db::use(data.value),
    db::use(data.num);

  for (size_t i(0); i < fn.parameters.size(); ++i)
  {
    std::shared_ptr<csnap::FunctionParameter> fp = fn.parameters.at(i);

    data.symbol_id = fn.dbid;
    data.is_template = false;
    data.name = fp->name;
    data.type = fp->type.toString();
    data.value = fp->default_value.repr;
    data.num = static_cast<int>(i);

    insert_into_db(*fp, db, query);
  }
}

void insert_template_params_into_database(DatabaseSession& db, const csnap::FunctionTemplate& fn)
{
  dto::FunctionParameter data;

  SqlStatement query{ db };

  query << "INSERT INTO parameter (symbol_sid, isTemplate, name, type, value, num) "
    "VALUES (?,?,?,?,?,?)",
    db::use(data.symbol_id),
    db::use(data.is_template),
    db::use(data.name),
    db::use(data.type),
    db::use(data.value),
    db::use(data.num);

  for (size_t i(0); i < fn.templateParameters().size(); ++i)
  {
    std::shared_ptr<csnap::TemplateParameter> tparam = fn.templateParameters().at(i);

    // @TODO: this code is duplicated, refactor! 

    data.symbol_id = fn.dbid;
    data.is_template = true;
    data.name = tparam->name;

    if (tparam->isTypeParameter())
    {
      data.type = "class";
      data.value = tparam->defaultValue<csnap::TemplateTypeParameter>().toString();
    }
    else
    {
      data.type = tparam->nontypeParameter().type.toString();
      data.value = tparam->defaultValue<csnap::TemplateNonTypeParameter>();
    }

    data.num = static_cast<int>(i);

    insert_into_db(*tparam, db, query);
  }
}

void insert_symbol_into_database(DatabaseSession& dbs, const csnap::Symbol& sym)
{
  dto::Symbol data;
  data.id = sym.dbid;
  data.what = static_cast<int>(sym.whatsit());
  data.parent = sourceview::parent_dbid(sym);
  data.name = sym.name;
  data.access = sourceview::access_dbvalue(sym);

  switch (sym.kind())
  {
  case csnap::Whatsit::Class:
  case csnap::Whatsit::ClassTemplate:
  {
    const auto& c = static_cast<const csnap::Class&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name, access, isStruct, isFinal) "
      "VALUES (?,?,?,?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name),
      db::use(data.access),
      db::use(data.is_struct),
      db::use(data.is_final);

    data.is_struct = c.is_struct;
    data.is_final = c.is_final;

    query.execute();
  }
  break;
  case csnap::Whatsit::Function:
  case csnap::Whatsit::FunctionTemplate:
  {
    const auto& f = static_cast<const csnap::Function&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name, access, isInline, isStatic, isVirtual, isFinal, isOverride, isConst, isConstexpr, isExplicit, isNoexcept, isDefault, isDelete, type) "
      "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name),
      db::use(data.access),
      db::use(data.is_inline),
      db::use(data.is_static),
      db::use(data.is_virtual),
      db::use(data.is_final),
      db::use(data.is_override),
      db::use(data.is_const),
      db::use(data.is_constexpr),
      db::use(data.is_explicit),
      db::use(data.is_noexcept),
      db::use(data.is_default),
      db::use(data.is_delete),
      db::use(data.type);

    data.is_inline = f.isInline();
    data.is_static = f.isStatic();
    data.is_virtual = f.isVirtual();
    data.is_final = f.isFinal();
    data.is_override = f.isOverride();
    data.is_const = f.isConst();
    data.is_constexpr = f.isConstexpr();
    data.is_explicit = f.isExplicit();
    data.is_noexcept = f.isNoexcept();
    data.is_default = f.isDefault();
    data.is_delete = f.isDelete();
    data.type = f.return_type.toString();

    query.execute();

    insert_fparam_into_database(dbs, f);

    if (f.is<csnap::FunctionTemplate>())
      insert_template_params_into_database(dbs, static_cast<const csnap::FunctionTemplate&>(f));
  }
  break;
  case csnap::Whatsit::Namespace:
  {
    const auto& ns = static_cast<const csnap::Namespace&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name) "
      "VALUES (?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name);

    query.execute();
  }
  break;
  case csnap::Whatsit::Enum:
  {
    const auto& enm = static_cast<const csnap::Enum&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name, access, isScoped) "
      "VALUES (?,?,?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name),
      db::use(data.access),
      db::use(data.is_scoped);

    data.is_scoped = enm.enum_class;

    query.execute();
  }
  break;
  case csnap::Whatsit::EnumValue:
  {
    const auto& enm = static_cast<const csnap::EnumValue&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name) "
      "VALUES (?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name);

    query.execute();
  }
  break;
  case csnap::Whatsit::Variable:
  {
    const auto& var = static_cast<const csnap::Variable&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name, access, isStatic, isInline, isConstexpr, type, value) "
      "VALUES (?,?,?,?,?,?,?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name),
      db::use(data.access),
      db::use(data.is_static),
      db::use(data.is_inline),
      db::use(data.is_constexpr),
      db::use(data.type),
      db::use(data.value);

    data.is_static = var.isStatic();
    data.is_inline = var.isInline();
    data.is_constexpr = var.isConstexpr();
    data.type = var.type().toString();
    data.value = var.defaultValue().repr;

    query.execute();
  }
  break;
  case csnap::Whatsit::Typedef:
  {
    const auto& tpd = static_cast<const csnap::Typedef&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name, access, type) "
      "VALUES (?,?,?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name),
      db::use(data.access),
      db::use(data.type);

    data.type = tpd.typedef_type.toString();

    query.execute();
  }
  break;
  case csnap::Whatsit::TypeAlias:
  {
    const auto& talias = static_cast<const csnap::TypeAlias&>(sym);

    SqlStatement query{ dbs };

    query << "INSERT INTO symbol (sid, parent, what, name, access, type) "
      "VALUES (?,?,?,?,?,?)",
      db::use(data.id),
      db::use(data.parent),
      db::use(data.what),
      db::use(data.name),
      db::use(data.access),
      db::use(data.type);

    data.type = talias.aliased_type.toString();

    query.execute();
  }
  break;
  default:
    break;
  }
}

void insert_symbols_into_database(DatabaseSession& db, const std::vector<std::shared_ptr<csnap::Symbol>>& symbols)
{
  for (const auto& s : symbols)
  {
    insert_symbol_into_database(db, *s);
  }
}

static bool has_new_fparam(const csnap::Function& fn)
{
  using ParamPtr = std::shared_ptr<csnap::FunctionParameter>;
  return !fn.parameters.empty() &&
    std::all_of(fn.parameters.begin(), fn.parameters.end(), [](ParamPtr p) -> bool {
    return p->dbid == -1;
      });
}

static void update_fparam_database(DatabaseSession& db, const csnap::Function& fn)
{
  // check if function had no parameters and now has some, 
  // in which case we must add the parameters instead of updating them
  if (has_new_fparam(fn))
  {
    insert_fparam_into_database(db, fn);
    return;
  }

  dto::FunctionParameter data;
  SqlStatement query{ db };

  query << "UPDATE parameter SET name = ?, value = ? "
    "WHERE id = ?",
    db::use(data.name),
    db::use(data.value),
    db::use(data.id);

  for (size_t i(0); i < fn.parameters.size(); ++i)
  {
    std::shared_ptr<csnap::FunctionParameter> fp = fn.parameters.at(i);

    data.id = fp->dbid;
    data.name = fp->name;
    data.value = fp->default_value.repr;

    size_t n = query.execute();
    assert(n == 1), (void)n;
  }
}

static void delete_template_params(DatabaseSession& dbs, const csnap::Symbol& symbol)
{
  int id = symbol.dbid;
  dbs << "DELETE FROM parameter WHERE symbol_sid = ? AND isTemplate = 1",
    db::use(id), db::now;
}

static void update_params_database(DatabaseSession& db, const csnap::ClassTemplate& c)
{
  delete_template_params(db, c);

  dto::FunctionParameter data;

  SqlStatement query{ db };

  query << "INSERT INTO parameter (symbol_sid, isTemplate, name, type, value, num) "
    "VALUES (?,?,?,?,?,?)",
    db::use(data.symbol_id),
    db::use(data.is_template),
    db::use(data.name),
    db::use(data.type),
    db::use(data.value),
    db::use(data.num);

  for (size_t i(0); i < c.templateParameters().size(); ++i)
  {
    std::shared_ptr<csnap::TemplateParameter> tparam = c.templateParameters().at(i);

    data.symbol_id = c.dbid;
    data.is_template = true;
    data.name = tparam->name;

    if (tparam->isTypeParameter())
    {
      data.type = "class";
      data.value = tparam->defaultValue<csnap::TemplateTypeParameter>().toString();
    }
    else
    {
      data.type = tparam->nontypeParameter().type.toString();
      data.value = tparam->defaultValue<csnap::TemplateNonTypeParameter>();
    }

    data.num = static_cast<int>(i);

    size_t n = query.execute();
    assert(n == 1), (void)n;
  }
}

void update_symbol_database(DatabaseSession& dbs, const csnap::Symbol& sym)
{
  dto::Symbol data;

  switch (sym.kind())
  {
  case csnap::Whatsit::Class:
  {
    const auto& c = static_cast<const csnap::Class&>(sym);

    SqlStatement query{ dbs };

    query << "UPDATE symbol SET isStruct = ?, isFinal = ? "
      "WHERE sid = ?",
      db::use(data.is_struct),
      db::use(data.is_final),
      db::use(data.id);

    data.id = c.dbid;
    data.is_struct = c.is_struct;
    data.is_final = c.is_final;

    size_t n = query.execute();
    assert(n == 1), (void)n;
  }
  break;
  case csnap::Whatsit::ClassTemplate:
  {
    const auto& c = static_cast<const csnap::ClassTemplate&>(sym);

    SqlStatement query{ dbs };

    query << "UPDATE symbol SET isStruct = ?, isFinal = ? "
      "WHERE sid = ?",
      db::use(data.is_struct),
      db::use(data.is_final),
      db::use(data.id);

    data.id = c.dbid;
    data.is_struct = c.is_struct;
    data.is_final = c.is_final;

    size_t n = query.execute();
    assert(n == 1), (void)n;

    update_params_database(dbs, c);
  }
  break;
  case csnap::Whatsit::Function:
  {
    const auto& f = static_cast<const csnap::Function&>(sym);

    SqlStatement query{ dbs };

    query << "UPDATE symbol SET isInline = ?, isStatic = ?, isVirtual = ?, isFinal = ?, "
      "isOverride = ?, isConst = ?, isConstexpr = ?, isExplicit = ?, isNoexcept = ?, "
      "isDefault = ?, isDelete = ? "
      "WHERE sid = ?",
      db::use(data.is_inline),
      db::use(data.is_static),
      db::use(data.is_virtual),
      db::use(data.is_final),
      db::use(data.is_override),
      db::use(data.is_const),
      db::use(data.is_constexpr),
      db::use(data.is_explicit),
      db::use(data.is_noexcept),
      db::use(data.is_default),
      db::use(data.is_delete),
      db::use(data.id);

    data.id = f.dbid;
    data.is_inline = f.isInline();
    data.is_static = f.isStatic();
    data.is_virtual = f.isVirtual();
    data.is_final = f.isFinal();
    data.is_override = f.isOverride();
    data.is_const = f.isConst();
    data.is_constexpr = f.isConstexpr();
    data.is_explicit = f.isExplicit();
    data.is_noexcept = f.isNoexcept();
    data.is_default = f.isDefault();
    data.is_delete = f.isDelete();

    size_t n = query.execute();
    assert(n == 1), (void)n;

    update_fparam_database(dbs, f);
  }
  break;
  case csnap::Whatsit::Namespace:
    break;
  case csnap::Whatsit::Enum:
    break;
  case csnap::Whatsit::EnumValue:
    break;
  default:
    break;
  }
}

void update_symbol_database(DatabaseSession& db, const std::vector<std::shared_ptr<csnap::Symbol>>& symbols)
{
  for (const auto& s : symbols)
  {
    update_symbol_database(db, *s);
  }
}

void insert_sema_into_database(DatabaseSession& db, const std::vector<csnap::SemaVariant>& semalist)
{
  dto::SymbolReference data;

  SqlStatement symref_query{ db };
  symref_query << "INSERT INTO symbolreference (symbol_sid, file_sid, line, col) "
    "VALUES (?,?,?,?)",
    db::use(data.symbol_id),
    db::use(data.file_id),
    db::use(data.line),
    db::use(data.col);

  SqlStatement symdef_query{ db };
  symdef_query << "INSERT INTO definition (symbol_sid, file_sid, line, col) "
    "VALUES (?,?,?,?)",
    db::use(data.symbol_id),
    db::use(data.file_id),
    db::use(data.line),
    db::use(data.col);

  SqlStatement symdecl_query{ db };
  symdecl_query << "INSERT INTO declaration (symbol_sid, file_sid, line, col) "
    "VALUES (?,?,?,?)",
    db::use(data.symbol_id),
    db::use(data.file_id),
    db::use(data.line),
    db::use(data.col);

  for (const csnap::SemaVariant& sema : semalist)
  {
    if (std::holds_alternative<csnap::SymbolReference>(sema))
    {
      const auto& symref = std::get<csnap::SymbolReference>(sema);

      data.symbol_id = symref.symbol_id;
      data.file_id = symref.file_id;
      data.line = symref.line;
      data.col = symref.col;

      symref_query.execute();
    }
    else if (std::holds_alternative<csnap::SymbolDeclaration>(sema))
    {
      const auto& symdecl = std::get<csnap::SymbolDeclaration>(sema);

      data.symbol_id = symdecl.symbol_id;
      data.file_id = symdecl.file_id;
      data.line = symdecl.line;
      data.col = symdecl.col;

      symdecl_query.execute();
    }
    else if (std::holds_alternative<csnap::SymbolDefinition>(sema))
    {
      const auto& symdef = std::get<csnap::SymbolDefinition>(sema);

      data.symbol_id = symdef.symbol_id;
      data.file_id = symdef.file_id;
      data.line = symdef.line;
      data.col = symdef.col;

      symdef_query.execute();
    }
  }
}

} // namespace sourceview

#endif