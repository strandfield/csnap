// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <csnap/database/snapshot.h>

#include <csnap/model/file.h>
#include <csnap/model/include.h>
#include <csnap/model/symbol.h>

#include <iostream>

int main(int argc, char* argv[])
{
  auto snap = csnap::Snapshot::create("testsnap.db");

  snap.setProperty("name", "toto");

  std::cout << snap.property("name") << std::endl;

  csnap::File exampleh;
  exampleh.path = "example.h";
  exampleh.id = snap.addFile(exampleh)->id;

  csnap::File examplecpp;
  examplecpp.path = "example.cpp";
  examplecpp.id = snap.addFile(examplecpp)->id;

  /*
  csnap::Include incl;
  incl.file_id = examplecpp.id.value();
  incl.included_file_id = exampleh.id.value();
  incl.line = 2;
  */

  csnap::Symbol empty{ csnap::Whatsit::CXXClass, "Empty" };
  empty.id = csnap::SymbolId(1);

  snap.addSymbols({ std::make_shared<csnap::Symbol>(empty) });

  csnap::SymbolReference def;
  def.symbol_id = empty.id.value();
  def.file_id = examplecpp.id.value();
  def.line = 5;
  def.col = 1;
  def.flags = csnap::SymbolReference::Definition;

  snap.addSymbolReferences({ def });
}
