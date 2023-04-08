// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_PAGEURL_H
#define CSNAP_PAGEURL_H

#include "xmlwriter.h"

#include <filesystem>

class PageURL
{
private:
  std::filesystem::path m_path;

public:
  PageURL(const PageURL&) = default;
  PageURL(PageURL&&) = default;

  PageURL(std::filesystem::path p);

  std::string pathToRoot() const;
  const std::filesystem::path& path() const;
  void setPath(const std::filesystem::path& fp);

  std::string linkTo(const std::filesystem::path& other) const;

  PageURL& operator=(const PageURL&) = default;
  PageURL& operator=(PageURL&&) = default;
};

inline PageURL::PageURL(std::filesystem::path p) :
  m_path(std::move(p))
{

}

inline std::string PageURL::pathToRoot() const
{
  std::string p = path().generic_string();
  size_t nbdir = std::count(p.begin(), p.end(), '/');

  std::string r;
  r.reserve(nbdir * 3);

  while (nbdir--)
  {
    r.append("../");
  }

  return r;
}

inline const std::filesystem::path& PageURL::path() const
{
  return m_path;
}

inline void PageURL::setPath(const std::filesystem::path& fp)
{
  m_path = fp;
}

inline std::string PageURL::linkTo(const std::filesystem::path& other) const
{
  std::filesystem::path current_dir = path().parent_path();
  std::filesystem::path relpath = std::filesystem::relative(other, current_dir);
  return relpath.generic_u8string();
}

#endif // CSNAP_PAGEURL_H
