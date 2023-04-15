// Copyright (C) 2023 Vincent Chambrin
// This file is part of the 'csnap' project.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include "exporter.h"

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
      std::string path = file.path.substr(rootdir.size());
      return std::filesystem::path(path + extension);
    }
    else
    {
      std::filesystem::path p{ file.path + extension };
      return p.relative_path();
    }
  }
};

SnapshotExporter::SnapshotExporter(Snapshot& s) :
  snapshot(s),
  outputdir(std::filesystem::current_path())
{

}

void SnapshotExporter::run()
{
  export_resources_html_assets(outputdir);

  DefinitionTable defs;
  defs.build(select_symboldefinition(snapshot.database()));

  SnapshotExporterHtmlPathResolver pathresolver;

  std::vector<File*> files = snapshot.files().all();

  for(size_t i(0); i < files.size(); ++i)
  {
    File* f = files.at(i);
    std::cout << "[" << (i+1) << "/" << files.size() << "] " << f->path << std::endl;
    export_html(snapshot, *f, outputdir, pathresolver.filePath(*f), defs, pathresolver);
  }

  writeSymbolPages();
}

void SnapshotExporter::writeSymbolPages()
{
  SymbolEnumerator symenumerator{ snapshot };

  while (symenumerator.next())
  {
    const Symbol& symbol = symenumerator.symbol;
   
    std::cout << symbol.display_name << std::endl;

    SnapshotExporterHtmlPathResolver pathresolver;
    std::string outputpath = "symbols/" + SourceHighlighter::symbol_symref(symbol) + ".html";
    export_symbol(snapshot, symbol, outputdir, outputpath, pathresolver);
  }
}

} // namespace csnap
