// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "filebrowser.h"

#include "xmlwriter.h"

namespace csnap
{

namespace html
{

void write_attributes(XmlWriter& xml, std::initializer_list<std::pair<std::string, std::string>>&& attrs)
{
  for (auto& p : attrs)
  {
    xml.writeAttribute(p.first, p.second);
  }
}

void startelement(XmlWriter& xml, const char* tagname, std::initializer_list<std::pair<std::string, std::string>>&& attrs)
{
  xml.writeStartElement(tagname);
  write_attributes(xml, std::move(attrs));
}

void endelement(XmlWriter& xml)
{
  xml.writeEndElement();
}

void link(XmlWriter& xml, std::initializer_list<std::pair<std::string, std::string>>&& attrs = {})
{
  startelement(xml, "link", std::move(attrs));
}

void script(XmlWriter& xml, std::initializer_list<std::pair<std::string, std::string>>&& attrs = {})
{
  startelement(xml, "script", std::move(attrs));
}

void div(XmlWriter& xml, std::initializer_list<std::pair<std::string, std::string>>&& attrs = {})
{
  startelement(xml, "div", std::move(attrs));
}

} // namespace html

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

    html::link(xml, {
      { "rel", "stylesheet" }, 
      { "type", "text/css" },
      { "href", rootPath() + "tooltip.css" }
    });
    html::endelement(xml);

    html::script(xml, { 
      { "type", "text/javascript" }
    });
    {
      xml.writeCharacters("var csnapRootPath=\"");
      xml.writeCharacters(rootPath());
      xml.writeCharacters("\";");
    }
    html::endelement(xml);

    html::script(xml, { 
      { "type", "text/javascript" }, 
      { "src", rootPath() + "codenav.js" } 
    });
    {
      xml.writeCharacters(" ");
    }
    html::endelement(xml);
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
  html::div(xml, { {"id", "content"} });
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
  html::endelement(xml);
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
