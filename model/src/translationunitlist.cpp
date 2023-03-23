// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "translationunitlist.h"

#include <algorithm>

namespace csnap
{

void TranslationUnitList::add(std::unique_ptr<TranslationUnit> tu)
{
  if (!tu)
    return;

  if (!tu->id.valid())
  {
    tu->id = TranslationUnitId((int)m_list.size());
  }

  size_t i = (size_t)tu->id.value();

  if (i >= m_list.size())
    m_list.resize(i + 1);

  m_list[i] = std::move(tu);
}

std::vector<TranslationUnit*> TranslationUnitList::all() const
{
  std::vector<TranslationUnit*> r;
  r.reserve(m_list.size());

  std::for_each(m_list.begin(), m_list.end(), [&r](const std::unique_ptr<TranslationUnit>& e) {
    if (e) r.push_back(e.get());
    });

  return r;
}

TranslationUnit* TranslationUnitList::get(Identifier<TranslationUnit> id) const
{
  if (!id.valid())
    return nullptr;

  size_t offset = (size_t)id.value();

  if (offset >= m_list.size())
    return nullptr;

  return m_list.at(offset).get();
}

TranslationUnit* TranslationUnitList::find(FileId fid) const
{
  auto it = std::find_if(m_list.begin(), m_list.end(), [&fid](const std::unique_ptr<TranslationUnit>& tu) {
    return tu && tu->sourcefile_id == fid;
    });

  return it != m_list.end() ? it->get() : nullptr;
}

} // namespace csnap
