// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_THREADPOOL_H
#define CSNAP_THREADPOOL_H

#include "queue.h"
#include "runnable.h"

#include <thread>
#include <vector>

namespace csnap
{

class ThreadPoolData;

/**
 * \brief a thread pool for running Runnable tasks
 */
class ThreadPool
{
public:
  explicit ThreadPool(size_t nb_threads = std::thread::hardware_concurrency());
  ThreadPool(const ThreadPool&) = delete;
  ~ThreadPool();

  void run(Runnable* r);

  size_t threadCount() const;
  void setThreadCount(size_t n);

  bool done() const;
  void waitForDone();

  ThreadPool& operator=(const ThreadPool&) = delete;

protected:
  void cleanupThreads();

private:
  std::vector<std::thread> m_threads;
  size_t m_nb_active_threads = 0;
  std::unique_ptr<ThreadPoolData> m_data;
};

} // namespace csnap

#endif // CSNAP_THREADPOOL_H
