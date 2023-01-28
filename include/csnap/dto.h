// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_DTO_H
#define CSNAP_DTO_H

#include <optional>
#include <string>

namespace csnap
{

namespace dto
{

struct Symbol
{
  int id;
  int what;
  std::optional<int> parent;
  std::string name;
  std::optional<int> access;
  int flags = 0;
  std::string type;
  std::string value;
};

struct FunctionParameter
{
  int id;
  int symbol_id;
  bool is_template;
  std::string name;
  std::string type;
  std::string value;
  int num;
};

struct File
{
  int id;
  std::string path;
  std::string content;
};

struct Include
{
  int file_id;
  int line;
  int included_file_id;
};

struct SymbolReference
{
  int symbol_id;
  int file_id;
  int line;
  int col;
};

} // namespace dto

} // namespace csnap

#endif // CSNAP_DTO_H
