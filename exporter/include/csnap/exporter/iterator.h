// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_ITERATOR_H
#define CSNAP_ITERATOR_H

#include <cassert>
#include <iterator>

/**
 * \brief a smart forward iterator that knows when end has been reached
 * 
 * This class simply stores an iterator and the end-iterator.
 */
template<typename It>
class Iterator
{
private:
  It m_iterator;
  It m_end;

public:

  using iterator_type = It;
  using value_type = typename It::value_type;

  Iterator() = delete;
  Iterator(const Iterator<It>&) = default;
  ~Iterator() = default;

  /**
   * \brief builds a smart iterator from a current and an end iterator
   * \param current  the current iterator
   * \param end      the end iterator
   * 
   * The \a end iterator should be reachable by moving the \a current iterator 
   * forward a finite number of times.
   */
  Iterator(It current, It end) :
    m_iterator(current),
    m_end(end)
  {
    assert(std::distance(current, end) >= 0);
  }

  /**
   * \brief returns the current value of the iterator
   */
  const It& iterator() const
  {
    return m_iterator;
  }

  /**
   * \brief returns the end iterator
   */
  const It& end() const
  {
    return m_end;
  }

  /**
   * \brief returns whether the iterator is at the end
   * 
   * If atend() is true, this iterator should not be dereferenced.
   */
  bool atend() const
  {
    return iterator() == end();
  }

  /**
   * \brief pre-increment operator
   * 
   * Advances the iterator by one, and returns a reference to the iterator.
   */
  Iterator<It>& operator++()
  {
    ++m_iterator;
    return *this;
  }

  /**
   * \brief post-increment operator
   * 
   * Advances the iterator by one, and returns a copy of the iterator before the 
   * increment.
   */
  Iterator<It> operator++(int)
  {
    Iterator<It> copy{ *this };
    ++(*this);
    return copy;
  }

  /**
   * \brief dereference the iterator
   */
  const value_type& operator*() const
  {
    return *m_iterator;
  }

  Iterator<It>& operator=(const Iterator<It>&) = default;
};

#endif // CSNAP_ITERATOR_H
