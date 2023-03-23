// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "sln.h"

#include "csnap/database/snapshot.h"

#include "csnap/model/file.h"
#include "csnap/model/translationunit.h"

#include <vcxproj/solution.h>
#include <vcxproj/utils.h>

namespace csnap
{

static void listFiles(Snapshot& ss, const vcxproj::Solution& solution)
{
  std::vector<File> files;

  for (const vcxproj::Project& p : solution.projects)
  {
    for (std::string f : p.includeList)
      files.push_back(create_file(std::move(f)));

    for (std::string f : p.compileList)
      files.push_back(create_file(std::move(f)));
  }

  ss.addFiles(files);
}

static void listTranslationUnits(Snapshot& ss, const vcxproj::Solution& solution)
{
  for (const vcxproj::Project& p : solution.projects)
  {
    program::CompileOptions params;

    const vcxproj::ItemDefinitionGroup& idg = p.itemDefinitionGroupList.back();

    std::vector<std::string> incdirs = vcxproj::split(idg.additionalIncludeDirectories, ';');

    for (const std::string& incdir : incdirs)
      params.includedirs.insert(incdir);

    std::vector<std::string> defines = vcxproj::split(idg.preprocessorDefinitions, ';');

    for (const std::string& def : defines)
    {
      size_t p = def.find('=');

      if (p != std::string::npos)
      {
        std::string name = def.substr(0, p);
        std::string value = def.substr(p + 1);
        params.defines[name] = value;
      }
      else
      {
        params.defines[def] = "";
      }
    }

    std::vector<FileId> list;
    list.reserve(p.compileList.size());

    for (std::string f : p.compileList)
    {
      File* file = ss.findFile(f);

      if (!file)
      {
        // @TODO: log some error
        continue;
      }

      list.push_back(file->id);
    }

    ss.addTranslationUnits(list, params);
  }
}

void openSln(const std::filesystem::path& path, Snapshot& snapshot)
{
  vcxproj::Solution solution = vcxproj::load_solution(path);

  snapshot.setProperty("sln.path", path.u8string());

  listFiles(snapshot, solution);
  listTranslationUnits(snapshot, solution);
}

} // namespace csnap
