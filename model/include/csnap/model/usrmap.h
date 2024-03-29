// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_USRMAP_H
#define CSNAP_USRMAP_H

#include "symbolid.h"

#include <map>
#include <memory>
#include <mutex>
#include <utility>

namespace csnap
{

/**
 * \brief maps clang USR to symbol identifier
 * 
 * libclang uses USRs (Unified Symbol Resolution) to match symbols across translation units.
 * This class is used to assign a SymbolId to a USR.
 */
class UsrMap
{
public:

  SymbolId find(const std::string& usr) const;
  SymbolId insert(const std::string& usr);
  void insert(const std::string& usr, SymbolId id);

private:
  std::map<std::string, SymbolId> m_map;
};

/**
 * \brief a thread-safe UsrMap
 */
class GlobalUsrMap
{
public:

  UsrMap clone();

  std::pair<SymbolId, bool> get(const std::string& usr);

protected:
  UsrMap& usrMap();
  std::mutex& mutex();

private:
  UsrMap m_map;
  std::mutex m_mutex;
};

} // namespace csnap

#endif // CSNAP_USRMAP_H
