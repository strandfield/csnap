// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILESEMA_H
#define CSNAP_FILESEMA_H

#include <csnap/model/file.h>
#include <csnap/model/include.h>
#include <csnap/model/reference.h>

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace csnap
{

/**
 * \brief provides semantic information about a file
 */
struct FileSema
{
  /**
   * \brief pointer to the file described by this struct
   */
  File* file = nullptr;

  /**
   * \brief the ordered list of files included by this file
   */
  std::vector<Include> includes;

  /**
   * \brief the ordered list of symbol references in this file
   */
  std::vector<SymbolReference> references;

  /**
   * \brief the list of files that are including this file
   */
  std::vector<Include> reverse_includes;
};

/**
 * \brief remove all references from a list that are implicit
 * \param refs  the list to clean up
 */
inline void remove_implicit_references(std::vector<SymbolReference>& refs)
{
  auto it = std::remove_if(refs.begin(), refs.end(), [](const SymbolReference& r) {
    return r.flags & SymbolReference::Implicit;
    });

  refs.erase(it, refs.end());
}

void simplify_ctor_and_class_references(std::vector<SymbolReference>& refs, const std::map<SymbolId, std::shared_ptr<Symbol>>& symbols);

} // namespace csnap

#endif // CSNAP_FILESEMA_H

