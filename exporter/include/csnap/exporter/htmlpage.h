// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_HTMLPAGE_H
#define CSNAP_HTMLPAGE_H

#include "filepagelinker.h"
#include "xmlwriter.h"

#include <filesystem>

class HtmlPage
{
private:
  csnap::FilePageLinker m_links;

public:
  XmlWriter& xml;

public:

  HtmlPage(PageURL purl, XmlWriter& xmlwriter);
  HtmlPage(const HtmlPage&) = delete;

  csnap::FilePageLinker& links();
  const csnap::FilePageLinker& links() const;
  const PageURL& url() const;

  std::string linkTo(const std::filesystem::path& otherPage) const;

  HtmlPage& operator=(const HtmlPage&) = delete;
};

inline HtmlPage::HtmlPage(PageURL purl, XmlWriter& xmlwriter) :
  m_links(std::move(purl)),
  xml(xmlwriter)
{

}

inline csnap::FilePageLinker& HtmlPage::links()
{
  return m_links;
}

inline const csnap::FilePageLinker& HtmlPage::links() const
{
  return m_links;
}

inline const PageURL& HtmlPage::url() const
{
  return links().currentPageUrl();
}

inline std::string HtmlPage::linkTo(const std::filesystem::path& otherPage) const
{
  return url().linkTo(otherPage);
}

#endif // CSNAP_HTMLPAGE_H
