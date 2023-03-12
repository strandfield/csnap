// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <csnap/database/snapshot.h>

#include <csnap/model/file.h>
#include <csnap/model/include.h>
#include <csnap/model/symbols.h>
#include <csnap/model/use.h>

#include <iostream>

int main(int argc, char* argv[])
{
  csnap::Snapshot snap;
  snap.create("testsnap.db");

  csnap::set_snapshot_info(snap, "name", "toto");

  std::cout << csnap::get_snapshot_info(snap, "name") << std::endl;

  csnap::File exampleh;
  exampleh.id = 1;
  exampleh.path = "example.h";

  csnap::insert_file(snap, exampleh);

  csnap::File examplecpp;
  examplecpp.id = 2;
  examplecpp.path = "example.cpp";

  csnap::insert_file(snap, examplecpp);

  csnap::Include incl;
  incl.file_id = examplecpp.id;
  incl.included_file_id = exampleh.id;
  incl.line = 2;

  csnap::insert_includes(snap, { incl });

  csnap::Class empty{ "Empty" };
  empty.id = 1;

  csnap::write_symbol(snap, empty);

  csnap::SymbolDefinition def;
  def.symbol_id = empty.id;
  def.file_id = examplecpp.id;
  def.line = 5;
  def.col = 1;

  csnap::insert_symbol_uses(snap, { def });
}
