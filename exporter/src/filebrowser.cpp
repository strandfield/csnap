// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filebrowser.h"

#include "htmlpage.h"

namespace csnap
{

FileBrowserGenerator::FileBrowserGenerator(HtmlPage& p, const FileContent& fc, FileSema fm, const FileList& fs, const SymbolMap& ss, const DefinitionTable& defs) :
  SourceHighlighter(p, fc, fm, fs, ss, defs)
{
}

void FileBrowserGenerator::generatePage()
{
  page.write("<!DOCTYPE html>\n");

  html::start(page);
  {
    html::head(page);
    {
      html::title(page);
      page << sema.file->path;
      html::endtitle(page);

      html::link(page, {
        { "id", "codestylesheet" },
        { "rel", "stylesheet" },
        { "type", "text/css" },
        { "href", page.url().pathToRoot() + "syntax.qtcreator.css" }
        });
      html::endlink(page);

      html::link(page, {
        { "rel", "stylesheet" },
        { "type", "text/css" },
        { "href", page.url().pathToRoot() + "tooltip.css" }
        });
      html::endlink(page);

      html::script(page, {
        { "type", "text/javascript" }
        });
      page << "var csnapRootPath=\"" << page.url().pathToRoot() << "\";";
      html::endscript(page);

      html::script(page, {
        { "type", "text/javascript" },
        { "src", page.url().pathToRoot() + "codenav.js" }
        });
      page << " ";
      html::endscript(page);
    }
    html::endhead(page);

    html::body(page);
    {
      generate();
    }
    html::endbody(page);
  }
  html::end(page);
}

void FileBrowserGenerator::generate()
{
  html::div(page, { {"id", "content"} });
  {
    html::table(page, { {"class", "code"} });
    page.xml.writeAttribute("file-id", sema.file->id.value());
    {
      html::tbody(page);
      writeCode();
      html::endtbody(page);
    }
    html::endtable(page);
  }
  html::enddiv(page);
}

void FileBrowserGenerator::generate(const std::set<int>& lines)
{
  html::table(page, { {"class", "code"} });
  {
    html::tbody(page);

    for (int l : lines)
    {
      writeLine(l - 1);
    }

    html::endtbody(page);
  }

  html::endtable(page);
}

void FileBrowserGenerator::writeLine(size_t i)
{
  html::tr(page);

  {
    html::th(page, { {"id", "L" + std::to_string(i + 1)} });
    page << std::to_string(i + 1);
    html::endth(page);
  }

  {
    html::td(page);
    writeLineSource((int)i + 1);
    html::endtd(page);
  }

  html::endtr(page);
}

void FileBrowserGenerator::writeCode()
{
  const std::vector<std::string_view>& lines = content.lines;

  for (size_t i(0); i < lines.size(); ++i)
  {
    writeLine(i);
  }
}

} // namespace csnap
