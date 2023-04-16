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

/**
 * \brief returns the list of all translation units as a vector
 */
std::vector<TranslationUnit*> TranslationUnitList::all() const
{
  std::vector<TranslationUnit*> r;
  r.reserve(m_list.size());

  std::for_each(m_list.begin(), m_list.end(), [&r](const TranslationUnitPtr& e) {
    if (e) r.push_back(e.get());
    });

  return r;
}

/**
 * \brief get a translation unit by id
 * \param id  the id of the translation unit
 */
TranslationUnit* TranslationUnitList::get(Identifier<TranslationUnit> id) const
{
  if (!id.valid())
    return nullptr;

  size_t offset = (size_t)id.value();

  if (offset >= m_list.size())
    return nullptr;

  return m_list.at(offset).get();
}

/**
 * \brief find a translation unit given a source file id
 * \param id  the id of the source file
 */
TranslationUnit* TranslationUnitList::find(FileId fid) const
{
  auto it = std::find_if(m_list.begin(), m_list.end(), [&fid](const TranslationUnitPtr& tu) {
    return tu && tu->sourcefile_id == fid;
    });

  return it != m_list.end() ? it->get() : nullptr;
}

/**
 * \brief returns the number of translation units in the list
 */
size_t TranslationUnitList::count() const
{
  return std::count_if(m_list.begin(), m_list.end(), [](const TranslationUnitPtr& tu) {
    return tu != nullptr;
    });
}

} // namespace csnap
