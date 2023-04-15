// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_RUNNABLE_H
#define CSNAP_RUNNABLE_H

#include <memory>

namespace csnap
{

/**
 * \brief abstract class for a task that can be run on a ThreadPool 
 */
class Runnable
{
public:
  virtual ~Runnable();

  bool autoDelete() const;
  void setAutoDelete(bool d = true);

  virtual void run() = 0;

private:
  bool m_auto_delete = true;
};

struct RunnableDeleter
{
  void operator()(Runnable* r);
};

using RunnablePtr = std::unique_ptr<Runnable, RunnableDeleter>;

} // namespace csnap

#endif // CSNAP_RUNNABLE_H
