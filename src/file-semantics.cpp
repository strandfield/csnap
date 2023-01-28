// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "csnap/file-semantics.h"

namespace csnap
{

FileInclude::FileInclude(int l, int f)
  : lineno(l),
    fileid(f)
{

}

int FileInclude::line() const
{
  return lineno;
}

int FileInclude::file() const
{
  return fileid;
}

void FileSemantics::addInclude(int file_id, int line, int col)
{
  includes.push_back(FileInclude(line, file_id));
  sema.push_back(SemaFileInclude(line, col, file_id));
}

csnap::FileSemantics& get_filesemantics(std::shared_ptr<FileSemantics>& ptr)
{
  if (!ptr)
    ptr = std::make_shared<csnap::FileSemantics>();

  return *ptr;
}

int get_line(const FileSemaVariant& sema)
{
  return std::visit([](const auto& e) -> int { return e.get_line(); }, sema);
}

int get_col(const FileSemaVariant& sema)
{
  return std::visit([](const auto& e) -> int { return e.get_col(); }, sema);
}

int comp(const FileSemaVariant& lhs, const FileSemaVariant& rhs)
{
  int lhs_l = get_line(lhs);
  int lhs_c = get_col(lhs);
  int rhs_l = get_line(rhs);
  int rhs_c = get_col(rhs);

  int l_diff = lhs_l - rhs_l;

  if (l_diff != 0)
    return l_diff;

  return lhs_c - rhs_c;
}

} // namespace csnap
