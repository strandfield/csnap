// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <csnap/snapshot.h>

#include <iostream>

int main(int argc, char* argv[])
{
  csnap::Snapshot snap;
  snap.create("testsnap.db");

  csnap::set_snapshot_info(snap, "name", "toto");

  std::cout << csnap::get_snapshot_info(snap, "name") << std::endl;
}
