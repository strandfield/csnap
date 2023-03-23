// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "threadpool.h"

namespace csnap
{

class ThreadPoolData
{
public:
  SharedQueue<RunnablePtr> tasks;
  size_t nb_active_tasks = 0;
  std::mutex mutex;
  std::condition_variable cv;
};

static void thread_pool_proc(ThreadPoolData& pooldata)
{
  for (;;)
  {
    RunnablePtr task{ pooldata.tasks.next() };

    if (!task)
      return;

    task->run();

    {
      std::lock_guard lock{ pooldata.mutex };
      --pooldata.nb_active_tasks;
    }

    pooldata.cv.notify_one();
  }
}

ThreadPool::ThreadPool(size_t nb_threads) : 
  m_nb_active_threads(0),
  m_data(std::make_unique<ThreadPoolData>())
{
  setThreadCount(nb_threads);
}

ThreadPool::~ThreadPool()
{
  {
    std::lock_guard lock{ m_data->mutex };
    // $TODO: check that there are no nullptr tasks in the queue #correctness
    m_data->nb_active_tasks -= m_data->tasks.size();
    m_data->tasks.clear();
  }

  for (size_t i(0); i < threadCount(); ++i)
  {
    m_data->tasks.write(nullptr);
  }

  for (std::thread& t : m_threads)
    t.join();
}

void ThreadPool::run(Runnable* r)
{
  std::lock_guard lock{ m_data->mutex };
  m_data->tasks.write(RunnablePtr(r));
  m_data->nb_active_tasks += 1;
}

size_t ThreadPool::threadCount() const
{
  // $Warning: this is not m_threads.size()
  return m_nb_active_threads;
}

void ThreadPool::setThreadCount(size_t n)
{
  cleanupThreads();

  while (n < m_nb_active_threads)
  {
    m_data->tasks.write(nullptr);
    --m_nb_active_threads;
  }

  while (m_nb_active_threads < n)
  {
    m_threads.push_back(std::thread(thread_pool_proc, std::ref(*m_data)));
    ++m_nb_active_threads;
  }
}

bool ThreadPool::done() const
{
  std::lock_guard lock{ m_data->mutex };
  return m_data->nb_active_tasks == 0;
}

void ThreadPool::waitForDone()
{
  std::unique_lock lock{ m_data->mutex };

  m_data->cv.wait(lock, [&]() {
    return m_data->nb_active_tasks == 0;
    });
}

void ThreadPool::cleanupThreads()
{
  auto it = std::remove_if(m_threads.begin(), m_threads.end(), [](const std::thread& t) {
    return !t.joinable();
    });

  m_threads.erase(it, m_threads.end());
}

} // namespace csnap
