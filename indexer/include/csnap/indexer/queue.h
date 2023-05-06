// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_QUEUE_H
#define CSNAP_QUEUE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>

namespace csnap
{

namespace details
{

struct SharedQueueSynchronizationData
{
  std::mutex mutex;
  std::condition_variable cv;
};

} // namespace details

/**
 * \brief a thread-safe queue
 * 
 * All public functions in this class are thread-safe.
 * Please note, however, that the class was designed for a multiple-producers /
 * single-consumer architecture; its API is therefore not that practical 
 * in a multiple-consumers scenario.
 */
template<typename T>
class SharedQueue
{
public:

  typedef T element_type;

  static_assert(std::is_default_constructible<element_type>::value, "SharedQueue::element_type must be default-constructible");
  static_assert(std::is_move_constructible<element_type>::value, "SharedQueue::element_type must be move-constructible");

  SharedQueue() :
    m_synchronization(std::make_unique<details::SharedQueueSynchronizationData>())
  {

  }

  SharedQueue(const SharedQueue&) = delete;

  /**
   * \brief fetch and remove the next element from the queue
   * 
   * If the queue is currently empty, the function waits until a new element 
   * is appended to the queue. 
   */
  T next()
  {
    std::unique_lock<std::mutex> lock{ mutex() };

    cv().wait(lock, [&]() {
      return !container().empty();
      });

    T n{ std::move(container().front()) };
    container().pop();
    return n;
  }

  /**
   * \brief wait for the next element
   * \param d  the maximum amount of time to wait
   * \return true if an element is available, false otherwise
   * 
   * If the queue isn't empty, this returns immediately.
   */
  bool waitForNext(std::chrono::milliseconds d)
  {
    std::unique_lock<std::mutex> lock{ mutex() };

    auto pred = [&]() {
      return !container().empty();
    };

    if (pred())
      return true;

    cv().wait_for(lock, d, pred);

    return pred();
  }

  /**
   * \brief appends an element to the queue
   * \param val  the element
   * 
   * Writing an element may wake-up another thread waiting in next()
   * or waitForNext().
   */
  void write(T val)
  {
    {
      std::lock_guard<std::mutex> lock{ mutex() };
      container().push(std::move(val));
    }

    cv().notify_one();
  }

  /**
   * \brief returns whether the queue is empty
   * 
   * \warning If more than one thread is consuming elements from the queue, 
   * the caller should not assume that calling next() will return immediately 
   * after empty() returned true: indeed, another thread may have taken the 
   * last element of the queue in the meantime.
   */
  bool empty() const
  {
    std::lock_guard<std::mutex> lock{ mutex() };
    return container().empty();
  }

  /**
   * \brief returns the number of elements in the queue
   * 
   * \sa empty().
   */
  size_t size() const
  {
    std::lock_guard<std::mutex> lock{ mutex() };
    return container().size();
  }

  /**
   * \brief removes all elements from the queue
   */
  void clear()
  {
    std::lock_guard<std::mutex> lock{ mutex() };
    container() = {};
  }

protected:

  std::mutex& mutex() const
  {
    return m_synchronization->mutex;
  }

  std::condition_variable& cv() const
  {
    return m_synchronization->cv;
  }

  std::queue<T>& container()
  {
    return m_queue;
  }

  const std::queue<T>& container() const
  {
    return m_queue;
  }

  SharedQueue<T> operator=(const SharedQueue<T>&) = delete;

private:
  std::queue<T> m_queue;
  std::unique_ptr<details::SharedQueueSynchronizationData> m_synchronization;
};

} // namespace csnap

#endif // CSNAP_QUEUE_H
