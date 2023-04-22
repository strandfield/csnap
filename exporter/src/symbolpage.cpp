// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "symbolpage.h"

#include "sourcehighlighter.h"

#include <cpptok/tokenizer.h>

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
  page.xml.write("<!DOCTYPE html>\n");

  html::start(page);
  {
    html::head(page);
    {
      html::title(page);
      page << symbol.name;
      html::endtitle(page);

      html::link(page, {
        { "rel", "stylesheet" },
        { "type", "text/css" },
        { "href", page.url().pathToRoot() + "symbolpage.css" }
        });
      html::endelement(page);
    }
    html::endlink(page);

    html::body(page);
    {
      writeBody();
    }
    html::endbody(page);
  }
  html::end(page);
}

void SymbolPageGenerator::writeBody()
{
  html::h1(page);
  page << symbol.name;
  html::endh1(page);

  writeSummary();

  std::vector<SymbolReference> refs = snapshot.listReferences(symbol.id);
  std::vector<SymbolReference> decls, defs;
  extract_decl_and_defs(refs, decls, defs);

  if (!decls.empty())
  {
    html::h2(page);
    page << "Declarations (" << (int)decls.size() << ")";
    html::endh2(page);

    writeDecls(decls);
  }

  if (!defs.empty())
  {
    html::h2(page);

    if (defs.size() > 1)
    {
      page << "Definitions (" << (int)defs.size() << ")";
    }
    else
    {
      page << "Definition";
    }
    html::endh2(page);

    writeDecls(defs);
  }

  if (!refs.empty())
  {
    html::h2(page);
    page << "Uses (" << (int)refs.size() << ")";
    html::endelement(page);

    writeUses(refs);
  }
}

void SymbolPageGenerator::writeSummary()
{
  html::p(page);
  {
    static const std::map<Symbol::Flag, std::string> dict = {
      { Symbol::Public, "public" },
      { Symbol::Protected, "protected" },
      { Symbol::Private, "private" },
      { Symbol::Inline, "inline" },
      { Symbol::Static, "static" },
      { Symbol::Constexpr, "constexpr" },
      { Symbol::IsScoped, "scoped" },
      { Symbol::IsStruct, "struct" },
      { Symbol::IsFinal, "final" },
      { Symbol::Virtual, "virtual" },
      { Symbol::Override, "override" },
      { Symbol::Final, "final" },
      { Symbol::Const, "const" },
      { Symbol::Pure, "pure" },
      { Symbol::Noexcept, "noexcept" },
      { Symbol::Explicit, "explicit" },
      { Symbol::Default, "default" },
      { Symbol::Delete, "delete" },
    };

    for (const auto& p : dict)
    {
      if (csnap::test_flag(symbol, p.first))
      {
        html::span(page);
        html::attr(page, "class", "symbol-flag");
        page << "[" << p.second << "]";
        html::endspan(page);
      }
    }
  }
  html::endp(page);

  if (symbol.parent_id.valid())
  {
    std::shared_ptr<Symbol> parent = snapshot.getSymbol(symbol.parent_id);

    if (parent)
    {
      html::p(page);
      {
        page << "Semantic parent: ";

        html::a(page);
        html::attr(page, "href", SourceHighlighter::symbol_symref(*parent) + ".html");
        page << parent->name;
        html::enda(page);
      }
      html::endp(page);
    }
  }

  if (symbol.kind == Whatsit::CXXClass || symbol.kind == Whatsit::Struct)
  {
    writeBases();
  }

  page.write("<p><b>Usr:</b> " + symbol.usr + "</p>\n");
}

void SymbolPageGenerator::writeBases()
{
  std::vector<BaseClass> bases = snapshot.listBaseClasses(symbol.id);

  if (bases.empty())
    return;

  html::p(page);
  {
    page << "Bases: ";

    for (size_t i(0); i < bases.size(); ++i)
    {
      const BaseClass& base = bases.at(i);
      std::shared_ptr<Symbol> basesymbol = snapshot.getSymbol(base.base_id);

      if (!basesymbol)
        continue;

      html::a(page);

      switch (base.access_specifier)
      {
      case AccessSpecifier::Public:
        html::attr(page, "class", "public-base");
        break;
      case AccessSpecifier::Protected:
        html::attr(page, "class", "protected-base");
        break;
      case AccessSpecifier::Private:
        html::attr(page, "class", "private-base");
        break;
      default:
        break;
      }

      html::attr(page, "href", SourceHighlighter::symbol_symref(*basesymbol) + ".html");

      page << basesymbol->name;
      html::enda(page);

      if (i != bases.size() - 1)
        page << ", ";
      else
        page << ".";
    }
  }
  html::endp(page);
}

void SymbolPageGenerator::writeDecls(const std::vector<SymbolReference>& list)
{
  for (const SymbolReference& d : list)
  {
    const File* f = snapshot.files().get(FileId(d.file_id));
    if (!f)
      continue;

    html::p(page);
    {
      std::string href = page.links().linkTo(*f, d.line);
      std::string text = f->path + ":" + std::to_string(d.line);

      if (!href.empty())
      {
        html::a(page, { { "href", href } });
        page << text;
        html::enda(page);
      }
      else
      {
        page << text;
      }
    }
    html::endp(page);
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

  std::shared_ptr<FileContent> content = snapshot.getFileContent(f->id);

  if (!content)
    return;

  html::h3(page);
  page << f->path << " (" << (int)std::distance(begin, end) << ")";
  html::endh3(page);

  /*SourceHighlighter highlighter{ page.xml };

  for (auto it = begin; it != end; ++it)
  {
    const SymbolReference& ref = *it;

    page.xml.writeStartElement("div");
    page.xml.writeAttribute("class", "code");
    highlighter.writeLineSource(ref.line);
    page.xml.writeEndElement();
  }*/


  cpptok::Tokenizer lexer;

  for (auto it = begin; it != end; ++it)
  {
    const SymbolReference& ref = *it;
    std::string_view linetext = content->lines.at(ref.line - 1);

    html::div(page, { {"class", "use"} });
    {
      html::div(page, { {"class", "ln"} });
      {
        html::a(page, { { "href", page.links().linkTo(*f, ref.line) } });
        page << std::to_string(ref.line);
        html::enda(page);
      }
      html::enddiv(page);

      html::div(page, { {"class", "code"} });

      if (ref.flags & SymbolReference::Implicit)
      {
        page << linetext;
      }
      else
      {
        size_t col = static_cast<size_t>(ref.col) - 1;
        page << linetext.substr(0, col);

        lexer.tokenize(linetext.data() + col, linetext.size() - col);

        html::span(page, { {"class", "here"} });
        page << lexer.output.front().text();
        html::endspan(page);

        page << linetext.substr(col + lexer.output.front().text().size());

        lexer.output.clear();
      }
      html::enddiv(page);
    }
    html::enddiv(page);
  }
}

} // namespace csnap
