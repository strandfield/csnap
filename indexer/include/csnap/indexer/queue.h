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

  void waitForNext(std::chrono::milliseconds d)
  {
    std::unique_lock<std::mutex> lock{ mutex() };

    auto pred = [&]() {
      return !container().empty();
    };

    if (pred())
      return;

    cv().wait_for(lock, d, pred);
  }

  // $TODO: should be written with a single lock #correctness
  T next(std::chrono::milliseconds wait)
  {
    if (empty())
      waitForNext(wait);

    return next();
  }

  void write(T val)
  {
    {
      std::lock_guard<std::mutex> lock{ mutex() };
      container().push(std::move(val));
    }

    cv().notify_one();
  }

  bool empty() const
  {
    std::lock_guard<std::mutex> lock{ mutex() };
    return container().empty();
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock{ mutex() };
    return container().size();
  }

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
