
#include "cli.h"

#include "csnap/indexer/scanner.h"

#include <iostream>

std::filesystem::path input(std::vector<std::string>& args)
{
  std::string path = read_arg(args, { "-i", "--input", "--sln" });
  
  std::filesystem::path r{ path };

  if (!std::filesystem::exists(r))
    throw std::runtime_error("input file does not exist");

  if(r.extension() != ".sln")
    throw std::runtime_error("input file is not a Visual Studio solution");

  return r;
}

std::filesystem::path output(std::vector<std::string>& args)
{
  std::string path = read_arg(args, { "-o", "--output" });

  std::filesystem::path r{ path };
  return r;
}

bool overwrite(std::vector<std::string>& args)
{
  return read_optional_flag(args, { "--overwrite" });
}

bool save_ast(std::vector<std::string>& args)
{
  return read_optional_flag(args, { "--save-ast" });
}

void scan(std::vector<std::string> args)
{
  using namespace csnap;

  Scanner scanner;
  scanner.save_ast = save_ast(args);

  std::filesystem::path dbpath = output(args);
  std::filesystem::path slnpath = input(args);
  bool should_overwrite = overwrite(args);

  if (!args.empty())
  {
    std::cerr << "unrecognized command line args: ";

    std::for_each(args.begin(), args.end(), [](const std::string& a) {
      std::cerr << a << " ";
      });

    std::cerr << std::endl;
    std::exit(1);
  }

  if (std::filesystem::exists(dbpath))
  {
    if (should_overwrite)
    {
      std::filesystem::remove(dbpath);
    }
    else
    {
      std::cerr << "output file already exists" << std::endl;
      std::exit(1);
    }
  }

  scanner.initSnapshot(dbpath);
  scanner.scanSln(slnpath);
}
