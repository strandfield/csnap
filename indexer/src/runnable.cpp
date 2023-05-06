// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "runnable.h"

namespace csnap
{

Runnable::~Runnable()
{

}

bool Runnable::autoDelete() const
{
  return m_auto_delete;
}

void Runnable::setAutoDelete(bool d)
{
  m_auto_delete = d;
}

void RunnableDeleter::operator()(Runnable* r)
{
  if (r && r->autoDelete())
    delete r;
}

} // namespace csnap
