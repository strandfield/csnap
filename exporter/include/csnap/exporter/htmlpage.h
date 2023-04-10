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

  void write(const std::string& txt);

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

inline void HtmlPage::write(const std::string& txt)
{
  xml.stream() << txt;
}

template<typename T>
XmlWriter::OutputStream& operator<<(HtmlPage& page, T&& data)
{
  page.xml.writeCharacters(std::string());
  page.xml.stream() << data;
  return page.xml.stream();
}

namespace html
{

inline void write_attributes(XmlWriter& xml, std::initializer_list<std::pair<std::string, std::string>>&& attrs)
{
  for (auto& p : attrs)
  {
    xml.writeAttribute(p.first, p.second);
  }
}

inline void startelement(HtmlPage& page, const char* tagname, std::initializer_list<std::pair<std::string, std::string>>&& attrs = {})
{
  page.xml.writeStartElement(tagname);
  write_attributes(page.xml, std::move(attrs));
}

inline void endelement(HtmlPage& page)
{
  page.xml.writeEndElement();
}

inline void start(HtmlPage& page)
{
  startelement(page, "html");
}

inline void end(HtmlPage& page)
{
  endelement(page);
}

#define DECLARE_HTML_ELEMENT(elem) inline void elem(HtmlPage& page, std::initializer_list<std::pair<std::string, std::string>>&& attrs = {}) \
  { \
    startelement(page, #elem, std::move(attrs)); \
  } \
 inline void end##elem(HtmlPage& page) \
  { \
    endelement(page); \
  } 

DECLARE_HTML_ELEMENT(head)
DECLARE_HTML_ELEMENT(body)
DECLARE_HTML_ELEMENT(title)
DECLARE_HTML_ELEMENT(link)
DECLARE_HTML_ELEMENT(script)
DECLARE_HTML_ELEMENT(div)
DECLARE_HTML_ELEMENT(h1)
DECLARE_HTML_ELEMENT(h2)
DECLARE_HTML_ELEMENT(h3)
DECLARE_HTML_ELEMENT(p)
DECLARE_HTML_ELEMENT(a)
DECLARE_HTML_ELEMENT(span)
DECLARE_HTML_ELEMENT(table)
DECLARE_HTML_ELEMENT(tbody)
DECLARE_HTML_ELEMENT(tr)
DECLARE_HTML_ELEMENT(th)
DECLARE_HTML_ELEMENT(td)

#undef DECLARE_HTML_ELEMENT

} // namespace html

#endif // CSNAP_HTMLPAGE_H
