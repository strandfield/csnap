// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#ifndef CSNAP_FILECONTENT_H
#define CSNAP_FILECONTENT_H

#include "file.h"

#include <string>
#include <string_view>
#include <vector>

namespace csnap
{

class FileContent
{
public:
  File* file = nullptr;
  std::string content;
  std::vector<std::string_view> lines;

public:
  FileContent(File* f, std::string data);

protected:
  void splitContentIntoLines();
};

inline FileContent::FileContent(File* f, std::string data) :
  file(f),
  content(std::move(data))
{
  // $TODO: add mechanism to make sure splitting into lines makes sense
  splitContentIntoLines();
}

inline void FileContent::splitContentIntoLines()
{
  lines.reserve(content.size() / 60);

  size_t start = 0;
  
  while (start < content.size())
  {
    size_t end = content.find('\n', start);

    if (end == std::string::npos)
      end = content.size();

    auto line = std::string_view(content.data() + start, end - start);
    lines.push_back(line);

    start = end + 1;
  }
}

} // namespace csnap

#endif // CSNAP_FILECONTENT_H
