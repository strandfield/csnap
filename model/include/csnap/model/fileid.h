// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILEID_H
#define CSNAP_FILEID_H

#include "identifier.h"

namespace csnap
{

struct File;

using FileId = Identifier<File>;

} // namespace csnap

#endif // CSNAP_FILEID_H
