// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_IDENTIFIER_H
#define CSNAP_IDENTIFIER_H

namespace csnap
{

template<typename T>
class Identifier
{
private:
  int m_value = -1;
public:

  Identifier() = default;
  Identifier(const Identifier<T>&) = default;
  ~Identifier() = default;

  explicit Identifier(int val) :
    m_value(val)
  {

  }

  int value() const
  {
    return m_value;
  }

  bool valid() const
  {
    return value() >= 0;
  }

  Identifier<T>& operator=(const Identifier<T>&) = default;
};

template<typename T>
bool operator==(const Identifier<T>& lhs, const Identifier<T>& rhs)
{
  return lhs.value() == rhs.value();
}

template<typename T>
bool operator!=(const Identifier<T>& lhs, const Identifier<T>& rhs)
{
  return lhs.value() != rhs.value();
}

template<typename T>
bool operator<(const Identifier<T>& lhs, const Identifier<T>& rhs)
{
  return lhs.value() < rhs.value();
}

} // namespace csnap

#endif // CSNAP_IDENTIFIER_H
