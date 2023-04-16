// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "xmlwriter.h"

#include <cassert>

/**
 * \brief constructs a xml writer on an output stream
 * \param output  the output stream
 * 
 * The output stream must be derived from std::ostream. 
 * This makes this class suitable for both writing directly to a file 
 * or in memory in a string buffer.
 */
XmlWriter::XmlWriter(OutputStream& output)
  : m_output(output)
{

}

/**
 * \brief returns the writer's output stream
 */
XmlWriter::OutputStream& XmlWriter::stream() const
{
  return m_output;
}

/**
 * \brief write bytes to the output stream
 * 
 * \warning Unlike writeCharacters(), this function does not escape 
 * special XML characters like '<'.
 */
void XmlWriter::write(const std::string& txt)
{
  stream() << txt;
}

/**
 * \brief starts a new xml element
 * \param qname  the qualified named of the element
 */
void XmlWriter::writeStartElement(const std::string& qname)
{
  if (withinElement() && !current().attributes_closed)
    sealAttributes();

  m_output << "<" << qname;

  Element e;
  e.name = qname;

  m_elements.push_back(std::move(e));
}

/**
 * \brief closes the current element
 * 
 * Depending on whether the element has inner text or nested elements, this will 
 * either close the element with "/>" or with "</tagname>".
 */
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

/**
 * \brief add an attribute to the current element
 * \param name   the attribute's name
 * \param value  the attribute's value
 * 
 * \warning You must write all attributes of an element immediately after it was opened
 * with writeStartElement().
 * Specifically, all calls to writeAttribute() must come before calls to writeCharacters()
 * or writeStartElement() (which will open a new element).
 */
void XmlWriter::writeAttribute(const std::string& name, const std::string& value)
{
  assert(!current().attributes_closed);

  m_output << " " << name << "=\"" << value << "\"";
}

/**
 * \brief add an attribute to the current element
 * \param name   the attribute's name
 * \param value  the attribute's value
 * 
 * This converts the integer to a string an uses the other overload.
 */
void XmlWriter::writeAttribute(const std::string& name, int value)
{
  writeAttribute(name, std::to_string(value));
}

/**
 * \brief writes characters
 * \param text  the text to write
 * 
 * This function inserts text under the current element (if any).
 * Special XML characters, like '<' are escaped.
 * 
 * When writing text inside an XML element, this function should only be called 
 * after all attributes have been written using writeAttribute().
 */
void XmlWriter::writeCharacters(const std::string& text)
{
  writeCharacters(std::string_view(text));
}

/**
 * \overload void XmlWriter::writeCharacters(const std::string& text)
 */
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

/**
 * \overload void XmlWriter::writeCharacters(const std::string& text)
 */
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

/**
 * \overload void XmlWriter::writeCharacters(const std::string& text)
 */
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
