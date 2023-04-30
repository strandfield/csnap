// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_PAGEURL_H
#define CSNAP_PAGEURL_H

#include "xmlwriter.h"

#include <algorithm>
#include <filesystem>

/**
 * \brief helper class for representing url of html pages
 */
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

/**
 * \brief constructs a page url
 * \param p  the path
 */
inline PageURL::PageURL(std::filesystem::path p) :
  m_path(std::move(p))
{

}

/**
 * \brief returns a relative path to the root
 * 
 * Example: 
 * \code{.cpp}
 *   PageURL("mypage.html").pathToRoot() == "";
 *   PageURL("folder/nested/mypage.html").pathToRoot() == "../../";
 * \endcode
 */
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

/**
 * \brief returns the path of the page
 */
inline const std::filesystem::path& PageURL::path() const
{
  return m_path;
}

inline void PageURL::setPath(const std::filesystem::path& fp)
{
  m_path = fp;
}

/**
 * \brief computes a relative link to another page
 * \param other  the path to the other page
 */
inline std::string PageURL::linkTo(const std::filesystem::path& other) const
{
  std::filesystem::path current_dir = path().parent_path();
  std::filesystem::path relpath = std::filesystem::relative(other, current_dir);
  return relpath.generic_u8string();
}

#endif // CSNAP_PAGEURL_H
