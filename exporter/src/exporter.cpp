// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "exporter.h"

#include "directorypage.h"
#include "filebrowser.h"
#include "symbolpage.h"
#include "writefile.h"
#include "xmlwriter.h"

#include "csnap/database/sqlqueries.h"
#include "csnap/database/symbolloader.h"

#include <fstream>
#include <set>
#include <sstream>

#include <iostream>

extern void export_resources_html_assets(const std::filesystem::path& outdir);

namespace csnap
{

void export_html(Snapshot& snapshot, File& file, const std::filesystem::path& outputdir, const std::filesystem::path& outputpath, const DefinitionTable& defs, PathResolver& pathresolver)
{
  std::shared_ptr<FileContent> fc = snapshot.getFileContent(file.id);

  if (!fc)
    return;

  FileSema sema;
  sema.file = &file;
  sema.references = snapshot.listReferencesInFile(file.id);
  sema.includes = snapshot.listIncludesInFile(file.id);
  sema.reverse_includes = snapshot.findWhereFileIsIncluded(file.id);

  remove_implicit_references(sema.references);

  FileBrowserGenerator::SymbolMap symbols;

  {
    std::set<SymbolId> ids;

    for (const SymbolReference& ref : sema.references)
    {
      ids.insert(SymbolId(ref.symbol_id));

      if (ref.parent_symbol_id.valid())
      {
        ids.insert(ref.parent_symbol_id);
      }
    }

    snapshot.loadSymbols(ids, symbols);
  }

  simplify_ctor_and_class_references(sema.references, symbols);

  std::stringstream outstrstream;
  XmlWriter xml{ outstrstream };

  HtmlPage page{ outputpath, xml };
  page.links().setPathResolver(pathresolver);

  FileBrowserGenerator generator{ page, *fc, std::move(sema), snapshot.files(), symbols, defs };
  generator.generatePage();

  write_file(outputdir / outputpath, outstrstream.str());
}

static std::set<std::filesystem::path> list_directories(const std::map<File*, std::filesystem::path>& paths)
{
  std::set<std::filesystem::path> result;

  for (const auto& p : paths)
  {
    std::filesystem::path parent_path = p.second.parent_path();

    if (parent_path != std::filesystem::path())
      result.insert(parent_path);
  }

  return result;
}

static std::set<std::filesystem::path> list_directories_recursive(const std::map<File*, std::filesystem::path>& paths)
{
  std::set<std::filesystem::path> result = list_directories(paths);

  for (auto it = result.begin(); it != result.end(); )
  {
    std::filesystem::path parent_path = it->parent_path();

    if (parent_path != std::filesystem::path())
    {
      auto [newit, inserted] = result.insert(parent_path);

      if (inserted)
        it = newit;
      else
        ++it;
    }
    else
    {
      ++it;
    }
  }

  return result;
}

static void export_symbol(Snapshot& snapshot, const Symbol& symbol, const std::filesystem::path& outputdir, const std::filesystem::path& outputpath, PathResolver& pathresolver)
{
  std::stringstream outstrstream;
  XmlWriter xml{ outstrstream };

  HtmlPage page{ outputpath, xml };
  SymbolPageGenerator pagegen{ page, snapshot, symbol };

  pagegen.setPathResolver(pathresolver);

  pagegen.writePage();

  write_file(outputdir / outputpath, outstrstream.str());
}

class SnapshotExporterHtmlPathResolver : public PathResolver
{
public:
  std::string rootdir;

public:
  explicit SnapshotExporterHtmlPathResolver(std::string root = {}) :
    rootdir(std::move(root))
  {

  }

  std::filesystem::path filePath(const File& file) const override
  {
    static const std::string extension = ".html";

    if (!rootdir.empty() && file.path.find(rootdir) == 0)
    {
      std::string path = file.path.substr(rootdir.size() + 1);
      return std::filesystem::path(path + extension);
    }
    else
    {
      std::filesystem::path p{ file.path + extension };
      return p.relative_path();
    }
  }
};

/**
 * \brief constructs a snapshot exporter on a given snapshot
 * \param s  the snapshot
 * 
 * By default, the \a outputdir in which the html files are written is 
 * the current working dir.
 */
SnapshotExporter::SnapshotExporter(Snapshot& s) :
  snapshot(s),
  outputdir(std::filesystem::current_path()),
  rootpath("%auto%")
{

}

/**
 * \brief runs the exporting process
 */
void SnapshotExporter::run()
{
  if (rootpath == "%auto%")
    detectRootPath();

  export_resources_html_assets(outputdir);

  std::map<File*, std::filesystem::path> paths = writeFilePages();

  writeDirectoryPages(paths);

  writeSymbolPages();
}

void SnapshotExporter::detectRootPath()
{
  std::vector<File*> files = snapshot.files().all();

  if (files.empty())
    return;

  rootpath = files.front()->path;

  auto common_substr_len = [](const std::string& a, const std::string& b) -> size_t {
    size_t n = std::min(a.size(), b.size());
    size_t i = 0;
    while (i < n && a.at(i) == b.at(i)) ++i;
    return i;
  };

  for (File* f : files)
  {
    size_t n = common_substr_len(rootpath, f->path);
    rootpath.erase(rootpath.begin() + n, rootpath.end());
  }

  if (!rootpath.empty())
  {
    rootpath.pop_back();

    if (rootpath.size() == 2 && rootpath.back() == ':')
      rootpath = "";

    rootpath = std::filesystem::path(rootpath).parent_path().generic_string();
  }

  std::cout << "exporter root path detected: " << rootpath << std::endl;
}

std::map<File*, std::filesystem::path> SnapshotExporter::writeFilePages()
{
  DefinitionTable defs;
  defs.build(select_symboldefinition(snapshot.database()));

  SnapshotExporterHtmlPathResolver pathresolver{ rootpath };

  std::vector<File*> files = snapshot.files().all();

  std::map<File*, std::filesystem::path> result;
  
  for (size_t i(0); i < files.size(); ++i)
  {
    File* f = files.at(i);
    std::cout << "[" << (i + 1) << "/" << files.size() << "] " << f->path << std::endl;
    std::filesystem::path savepath = pathresolver.filePath(*f);
    result[f] = savepath;
    export_html(snapshot, *f, outputdir, savepath, defs, pathresolver);
  }

  return result;
}

void SnapshotExporter::writeDirectoryPages(const std::map<File*, std::filesystem::path>& paths)
{
  std::cout << "Directory pages:" << std::endl;

  std::set<std::filesystem::path> directories = list_directories_recursive(paths);

  // $note: ugly hack to get a homepage
  directories.insert(std::filesystem::path());

  for (const std::filesystem::path& dirpath : directories)
  {
    std::cout << dirpath.generic_string() << std::endl;

    std::stringstream outstrstream;
    XmlWriter xml{ outstrstream };

    HtmlPage page{ dirpath / "index.html", xml };
   
    DirectoryPageGenerator gen{ page, paths, dirpath };
    gen.writePage();

    write_file(outputdir / page.url().path(), outstrstream.str());
  }
}

void SnapshotExporter::writeSymbolPages()
{
  SymbolEnumerator symenumerator{ snapshot };

  while (symenumerator.next())
  {
    const Symbol& symbol = symenumerator.symbol;
   
    std::cout << symbol.display_name << std::endl;

    SnapshotExporterHtmlPathResolver pathresolver{ rootpath };
    std::string outputpath = "symbols/" + SourceHighlighter::symbol_symref(symbol) + ".html";
    export_symbol(snapshot, symbol, outputdir, outputpath, pathresolver);
  }
}

} // namespace csnap
