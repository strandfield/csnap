// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filebrowser.h"

#include "xmlwriter.h"

namespace csnap
{

FileBrowserGenerator::FileBrowserGenerator(XmlWriter& xmlstream, const FileContent& fc, FileSema fm, const FileList& fs, const SymbolMap& ss, const DefinitionTable& defs) :
  SourceHighlighter(xmlstream, fc, fm, fs, ss, defs)
{
}

void FileBrowserGenerator::generatePage()
{
  xml.stream() << "<!DOCTYPE html>\n";

  xml::Element html{ xml, "html" };

  {
    xml::Element head{ xml, "head" };

    {
      xml::Element title{ xml, "title" };
      title.text(sema.file->path);
    }

    {
      xml::Element link{ xml, "link" };
      link.attr("id", "codestylesheet");
      link.attr("rel", "stylesheet");
      link.attr("type", "text/css");
      link.attr("href", rootPath() + "syntax.qtcreator.css");
    }
  }

  {
    xml::Element body{ xml, "body" };

    {
      generate();
    }
  }
}

void FileBrowserGenerator::generate()
{
  xml.writeStartElement("table");
  xml.writeAttribute("class", "code");
  xml.writeAttribute("file-id", sema.file->id.value());

  {
    xml.writeStartElement("tbody");

    writeCode();

    xml.writeEndElement();
  }

  xml.writeEndElement();
}

void FileBrowserGenerator::generate(const std::set<int>& lines)
{
  xml.writeStartElement("table");
  xml.writeAttribute("class", "code");

  {
    xml.writeStartElement("tbody");

    for (int l : lines)
    {
      writeLine(l - 1);
    }

    xml.writeEndElement();
  }

  xml.writeEndElement();
}

void FileBrowserGenerator::writeLine(size_t i)
{
  xml.writeStartElement("tr");

  {
    xml.writeStartElement("th");
    xml.writeAttribute("id", "L" + std::to_string(i + 1));
    xml.writeCharacters(std::to_string(i + 1));
    xml.writeEndElement();
  }

  {
    xml.writeStartElement("td");

    writeLineSource((int)i + 1);

    xml.writeEndElement();
  }

  xml.writeEndElement();
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
