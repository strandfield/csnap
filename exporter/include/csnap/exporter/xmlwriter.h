// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_XMLWRITER_H
#define CSNAP_XMLWRITER_H

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

/**
 * \brief helper class to write xml files
 * 
 * This class provides a friendly interface to write XML files.
 * 
 * \note This class does not enforce compliance with any XML standard.
 * In particular, this class does not ensure that the resulting XML document 
 * has a single root element.
 */
class XmlWriter
{
public:
  using OutputStream = std::ostream;

  explicit XmlWriter(OutputStream& output);
  XmlWriter(const XmlWriter&) = delete;

  OutputStream& stream() const;

  void write(const std::string& txt);

  void writeStartElement(const std::string& qname);
  void writeEndElement();

  void writeAttribute(const std::string& name, const std::string& value);
  void writeAttribute(const std::string& name, int value);

  void writeCharacters(const std::string& text);
  void writeCharacters(std::string_view text);
  void writeCharacters(const char* str);
  void writeCharacters(const char* str, size_t n);

  XmlWriter& operator=(const XmlWriter&) = delete;

protected:

  struct Element
  {
    std::string name;
    bool attributes_closed = false;
  };

  bool withinElement() const;
  Element& current();
  void sealAttributes();

private:
  OutputStream& m_output;
  std::vector<Element> m_elements;
};

#endif // CSNAP_XMLWRITER_H
