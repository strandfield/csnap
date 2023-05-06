
#include "cli.h"

#include "csnap/exporter/exporter.h"

#include <iostream>

namespace
{

std::filesystem::path input(std::vector<std::string>& args)
{
  std::string path = read_arg(args, { "-i", "--input", "--snapshot" });

  std::filesystem::path r{ path };

  if (!std::filesystem::exists(r))
    throw std::runtime_error("input file does not exist");

  return r;
}

std::filesystem::path output(std::vector<std::string>& args)
{
  std::string path = read_arg(args, { "-o", "--output" });

  std::filesystem::path r{ path };
  return r;
}

} // namespace

void export_(std::vector<std::string> args)
{
  using namespace csnap;

  std::filesystem::path snapshot_path = input(args);

  auto snapshot = Snapshot::open(snapshot_path);

  SnapshotExporter exporter{ snapshot };
  exporter.outputdir = output(args);

  if (!std::filesystem::exists(exporter.outputdir))
    std::filesystem::create_directories(exporter.outputdir);

  if (!args.empty())
  {
    std::cerr << "unrecognized command line args: ";

    std::for_each(args.begin(), args.end(), [](const std::string& a) {
      std::cerr << a << " ";
      });

    std::cerr << std::endl;
    
    throw std::runtime_error("unrecognized command line args");
  }

  exporter.run();
}
