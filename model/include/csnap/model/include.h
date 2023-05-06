// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_INCLUDE_H
#define CSNAP_INCLUDE_H

#include "fileid.h"

namespace csnap
{

/**
 * \brief stores information about a file #include
 */
struct Include
{
  /**
   * \brief the id of the file in which the include directive appears
   */
  FileId file_id;

  /**
   * \brief the id of the file that is being included
   */
  FileId included_file_id;

  /**
   * \brief the line number of the include directive
   */
  int line = -1;
};

} // namespace csnap

#endif // CSNAP_INCLUDE_H
