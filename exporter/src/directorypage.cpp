// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "directorypage.h"

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

namespace csnap
{

struct DirectoryContent
{
  std::vector<std::pair<File*, std::filesystem::path>> files;
  std::set<std::filesystem::path> folders;
};

static DirectoryContent filter_directory_content(const std::map<File*, std::filesystem::path>& allpages, std::filesystem::path wd)
{
  std::string genwd = wd.generic_string();

  DirectoryContent result;

  for (const auto& p : allpages)
  {
    const std::string& path = p.second.generic_string();

    if (path.length() <= genwd.length() || std::strncmp(genwd.c_str(), path.c_str(), genwd.size()) != 0) // not in folder
      continue;

    if (std::count(path.begin() + genwd.size(), path.end(), '/') == 1) // in the folder
    {
      result.files.push_back(std::make_pair(p.first, p.second));
    }
    else // in a nested folder
    {
      std::filesystem::path folderpath = p.second.parent_path().string().substr(genwd.size() + (!genwd.empty() ? 1 : 0));
      result.folders.insert(folderpath);
    }
  }

  return result;
}

static void filter_folders(std::set<std::filesystem::path>& paths)
{
  using Path = std::filesystem::path;

  auto comp = [](const Path& lhs, const Path& rhs) {
    const std::string& a = lhs.generic_string();
    const std::string& b = lhs.generic_string();
    return a.size() < b.size() 
      && b.at(a.size()) == '/'
      && std::strncmp(a.c_str(), b.c_str(), a.size()) == 0;
  };

  auto it = std::adjacent_find(paths.begin(), paths.end(), comp);

  while (it != paths.end())
  {
    it = paths.erase(it);
    it = std::adjacent_find(it, paths.end(), comp);
  }
}

static void sort(DirectoryContent& dircontent)
{
  using P = std::pair<File*, std::filesystem::path>;

  std::sort(dircontent.files.begin(), dircontent.files.end(), [](const P& lhs, const P& rhs) {
    return lhs.second < rhs.second;
    });
}


static std::string get_filename(const std::string& path)
{
  size_t pos = path.rfind('/');
  return pos == std::string::npos ? path : path.substr(pos + 1);
}

DirectoryPageGenerator::DirectoryPageGenerator(HtmlPage& p, const std::map<File*, std::filesystem::path>& allpages, std::filesystem::path wd) :
  page(p),
  all_pages(allpages),
  working_dir(std::move(wd))
{

}

void DirectoryPageGenerator::writePage()
{
  page.xml.write("<!DOCTYPE html>\n");

  html::start(page);
  {
    html::head(page);
    {
      html::title(page);
      page << working_dir.generic_string();
      html::endtitle(page);
    }
    html::endhead(page);

    html::body(page);
    {
      writeBody();
    }
    html::endbody(page);
  }
  html::end(page);
}

void DirectoryPageGenerator::writeBody()
{
  html::h1(page);
  page << working_dir.generic_string();
  html::endh1(page);

  DirectoryContent content = filter_directory_content(all_pages, working_dir);
  filter_folders(content.folders);
  sort(content);

  html::table(page);
  {
    html::tbody(page);
    {
      for (const std::filesystem::path& p : content.folders)
      {
        std::string foldername = p.string();
        std::string href = foldername + "/index.html";

        html::tr(page);
        {
          html::td(page);
          {
            html::a(page);
            html::attr(page, "href", href);
            page << foldername;
            html::enda(page);
          }
          html::endtd(page);
        }
        html::endtr(page);
      }

      for (const auto& p : content.files)
      {
        std::string filename = get_filename(p.first->path);
        std::string href = std::filesystem::relative(p.second, working_dir).string();

        html::tr(page);
        {
          html::td(page);
          {
            html::a(page);
            html::attr(page, "href", href);
            page << filename;
            html::enda(page);
          }
          html::endtd(page);
        }
        html::endtr(page);
      }
    }
    html::endtbody(page);
  }
  html::endtable(page);
}


} // namespace csnap
