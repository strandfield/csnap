// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_ITERATOR_H
#define CSNAP_ITERATOR_H

#include <cassert>

template<typename It>
class Iterator
{
public:

  using iterator_type = It;
  using value_type = typename It::value_type;

  Iterator() = delete;
  Iterator(const Iterator<It>&) = default;

  Iterator(It begin, It end) :
    m_iterator(begin),
    m_end(end)
  {

  }

  It iterator() const
  {
    return m_iterator;
  }

  It end() const
  {
    return m_end;
  }

  bool atend() const
  {
    return iterator() == end();
  }

  Iterator<It>& operator++()
  {
    ++m_iterator;
    return *this;
  }

  Iterator<It> operator++(int)
  {
    SafeIterator<It> copy{ *this };
    ++(*this);
    return copy;
  }

  const value_type& operator*() const
  {
    return *m_iterator;
  }

  Iterator<It>& operator=(const Iterator<It>&) = default;

public:
  It m_iterator;
  It m_end;
};

#endif // CSNAP_ITERATOR_H
