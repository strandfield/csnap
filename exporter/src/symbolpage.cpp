// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "symbolpage.h"

#include "sourcehighlighter.h"

namespace csnap
{

void extract_decl_and_defs(std::vector<SymbolReference>& refs, std::vector<SymbolReference>& decls, std::vector<SymbolReference>& defs)
{
  auto is_decl_or_def = [](const SymbolReference& r) -> bool {
    return r.flags & (SymbolReference::Declaration | SymbolReference::Definition);
  };

  auto it = std::find_if(refs.begin(), refs.end(), is_decl_or_def);

  while (it != refs.end())
  {
    if (it->flags & SymbolReference::Declaration)
      decls.push_back(*it);
    else
      defs.push_back(*it);

    it = refs.erase(it);

    it = std::find_if(refs.begin(), refs.end(), is_decl_or_def);
  }
}

SymbolPageGenerator::SymbolPageGenerator(HtmlPage& p, Snapshot& snap, const Symbol& sym) :
  page(p),
  snapshot(snap),
  symbol(sym)
{

}

PathResolver* SymbolPageGenerator::pathResolver() const
{
  return page.links().pathResolver();
}

void SymbolPageGenerator::setPathResolver(PathResolver& resolver)
{
  page.links().setPathResolver(resolver);
}

void SymbolPageGenerator::writePage()
{
  page.xml.stream() << "<!DOCTYPE html>\n";

  xml::Element html{ page.xml, "html" };

  {
    xml::Element head{ page.xml, "head" };

    {
      xml::Element title{ page.xml, "title" };
      title.text(symbol.name);
    }

    {
      xml::Element link{ page.xml, "link" };
      link.attr("id", "codestylesheet");
      link.attr("rel", "stylesheet");
      link.attr("type", "text/css");
      link.attr("href", page.url().pathToRoot() + "syntax.qtcreator.css");
    }
  }

  {
    xml::Element body{ page.xml, "body" };

    {
      writeBody();
    }
  }
}

void SymbolPageGenerator::writeBody()
{
  {
    xml::Element h1{ page.xml, "h1" };
    h1.text(symbol.name);
  }

  {
    page.xml.stream() << "<p><b>Usr:</b> " << symbol.usr << "</p>\n";
  }

  std::vector<SymbolReference> refs = snapshot.listReferences(symbol.id);
  std::vector<SymbolReference> decls, defs;
  extract_decl_and_defs(refs, decls, defs);

  if (!decls.empty())
  {
    page.xml.writeStartElement("h2");

    page.xml.writeCharacters("Declarations (");
    page.xml.writeCharacters(std::to_string(decls.size()));
    page.xml.writeCharacters(")");

    page.xml.writeEndElement();

    writeDecls(decls);
  }

  if (!defs.empty())
  {
    page.xml.writeStartElement("h2");

    if (defs.size() > 1)
    {
      page.xml.writeCharacters("Definitions (");
      page.xml.writeCharacters(std::to_string(defs.size()));
      page.xml.writeCharacters(")");
    }
    else
    {
      page.xml.writeCharacters("Definition");
    }

    page.xml.writeEndElement();

    writeDecls(defs);
  }

  if (!refs.empty())
  {
    page.xml.writeStartElement("h2");

    page.xml.writeCharacters("Uses (");
    page.xml.writeCharacters(std::to_string(refs.size()));
    page.xml.writeCharacters(")");

    page.xml.writeEndElement();

    writeUses(refs);
  }
}

void SymbolPageGenerator::writeDecls(const std::vector<SymbolReference>& list)
{
  for (const SymbolReference& d : list)
  {
    const File* f = snapshot.files().get(FileId(d.file_id));
    if (!f)
      continue;

    page.xml.writeStartElement("p");

    {
      std::string href = page.links().linkTo(*f, d.line);
      std::string text = f->path + ":" + std::to_string(d.line);

      if (!href.empty())
      {
        page.xml.writeStartElement("a");
        page.xml.writeAttribute("href", href);
        page.xml.writeCharacters(text);
        page.xml.writeEndElement();
      }
      else
      {
        page.xml.writeCharacters(text);
      }
    }

    page.xml.writeEndElement();
  }
}

void SymbolPageGenerator::writeUses(const std::vector<SymbolReference>& list)
{
  auto begin = list.begin();

  while (begin != list.end())
  {
    auto it = std::find_if(begin, list.end(), [begin](const SymbolReference& r) {
      return r.file_id != begin->file_id;
      });

    writeUsesInFile(begin, it);

    begin = it;
  }
}

void SymbolPageGenerator::writeUsesInFile(RefIterator begin, RefIterator end)
{
  if (begin == end)
    return;

  const File* f = snapshot.files().get(FileId(begin->file_id));

  if (!f)
    return;

  page.xml.writeStartElement("h3");

  page.xml.writeCharacters(f->path);
  page.xml.writeCharacters(" (" + std::to_string(std::distance(begin, end)) + ")");

  page.xml.writeEndElement();

  /*SourceHighlighter highlighter{ page.xml };

  for (auto it = begin; it != end; ++it)
  {
    const SymbolReference& ref = *it;

    page.xml.writeStartElement("div");
    page.xml.writeAttribute("class", "code");
    highlighter.writeLineSource(ref.line);
    page.xml.writeEndElement();
  }*/

  page.xml.writeStartElement("ul");

  for (auto it = begin; it != end; ++it)
  {
    const SymbolReference& ref = *it;

    page.xml.writeStartElement("li");
    {
      page.xml.writeStartElement("a");
      page.xml.writeAttribute("href", page.links().linkTo(*f, ref.line));
      page.xml.writeCharacters("L");
      page.xml.writeCharacters(std::to_string(ref.line));
      page.xml.writeEndElement();
    }
    page.xml.writeEndElement();
  }

  page.xml.writeEndElement();
}

} // namespace csnap
