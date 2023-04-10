// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "xmlwriter.h"

#include <cassert>

XmlWriter::XmlWriter(OutputStream& output)
  : m_output(output)
{

}

XmlWriter::OutputStream& XmlWriter::stream() const
{
  return m_output;
}

void XmlWriter::write(const std::string& txt)
{
  stream() << txt;
}

void XmlWriter::writeStartElement(const std::string& qname)
{
  if (withinElement() && !current().attributes_closed)
    sealAttributes();

  m_output << "<" << qname;

  Element e;
  e.name = qname;

  m_elements.push_back(std::move(e));
}

void XmlWriter::writeEndElement()
{
  if (!m_elements.back().attributes_closed)
  {
    m_output << "/>";
  }
  else
  {
    m_output << "</" << m_elements.back().name << ">";
  }

  m_elements.pop_back();
}


void XmlWriter::writeAttribute(const std::string& name, const std::string& value)
{
  assert(!current().attributes_closed);

  m_output << " " << name << "=\"" << value << "\"";
}

void XmlWriter::writeAttribute(const std::string& name, int value)
{
  assert(!current().attributes_closed);

  m_output << " " << name << "=\"" << std::to_string(value) << "\"";
}

void XmlWriter::writeCharacters(const std::string& text)
{
  writeCharacters(std::string_view(text));
}

void XmlWriter::writeCharacters(std::string_view text)
{
  writeCharacters(text.data(), text.size());
}

inline void write_char_escaped(XmlWriter::OutputStream& stream, char c)
{
  // https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents


  if (c == '<')
    stream << "&lt;";
  else if (c == '>')
    stream << "&gt;";
  else if (c == '&')
    stream << "&amp;";
  else
    stream << c;
}

void XmlWriter::writeCharacters(const char* str)
{
  if (withinElement() && !current().attributes_closed)
    sealAttributes();

  while(*str)
  {
    char c = *(str++);
    write_char_escaped(m_output, c);
  }
}

void XmlWriter::writeCharacters(const char* str, size_t n)
{
  if (withinElement() && !current().attributes_closed)
    sealAttributes();

  while(n--)
  {
    char c = *(str++);
    write_char_escaped(m_output, c);
  }
}

bool XmlWriter::withinElement() const
{
  return !m_elements.empty();
}

XmlWriter::Element& XmlWriter::current()
{
  return m_elements.back();
}

void XmlWriter::sealAttributes()
{
  m_output << ">";

  current().attributes_closed = true;
}
