// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_TRANSLATIONUNITLIST_H
#define CSNAP_TRANSLATIONUNITLIST_H

#include "fileid.h"
#include "translationunit.h"

#include <memory>
#include <vector>

namespace csnap
{

class TranslationUnitList
{
public:

  void add(std::unique_ptr<TranslationUnit> tu);
  std::vector<TranslationUnit*> all() const;
  TranslationUnit* get(Identifier<TranslationUnit> id) const;

  TranslationUnit* find(FileId fid) const;

private:
  std::vector<std::unique_ptr<TranslationUnit>> m_list;

};

} // namespace csnap

#endif // CSNAP_TRANSLATIONUNITLIST_H
