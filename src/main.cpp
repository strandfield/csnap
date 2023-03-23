
#include "csnap/indexer/scanner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

std::string read_arg_val(std::vector<std::string>& args, std::vector<std::string>::iterator it)
{
  if (std::next(it) == args.end())
    throw std::runtime_error("missing value for cli option: " + *it);

  std::string val{ std::move(*std::next(it)) };
  args.erase(it, it + 2);
  return val;
}

std::string read_arg(std::vector<std::string>& args, const std::vector<std::string>& names)
{
  auto it = std::find_if(args.begin(), args.end(), [&names](const std::string& a) {
    return std::find(names.begin(), names.end(), a) != names.end();
    });

  if (it == args.end())
    throw std::runtime_error("missing cli option: " + names.front());

  return read_arg_val(args, it);
}

bool read_optional_flag(std::vector<std::string>& args, const std::vector<std::string>& names)
{
  auto it = std::find_if(args.begin(), args.end(), [&names](const std::string& a) {
    return std::find(names.begin(), names.end(), a) != names.end();
    });

  if (it == args.end())
    return false;

  args.erase(it);
  return true;
}

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

void scan(std::vector<std::string>& args)
{
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

  using namespace csnap;
  Scanner scanner;
  scanner.initSnapshot(dbpath);
  scanner.scanSln(slnpath);
}

[[noreturn]] void help()
{
  std::cout << "csnap is a libclang-based command-line utility to create snapshots of C++ programs." << std::endl;
  std::cout << std::endl;
  std::cout << "Syntax:" << std::endl;
  std::cout << "  csnap run --sln <Visual Studio solution> --ouput <snapshot.db>" << std::endl;

  std::exit(0);
}

int main(int argc, char* argv[])
{
  auto args = std::vector<std::string>(argv, argv + argc);

  if (args.size() < 2)
    help();

  if (args.at(1) == "scan")
  {
    args.erase(args.begin(), args.begin() + 2);
    scan(args);
  }
  else
  {
    std::cerr << "unrecognized command " << args.at(1) << std::endl;
    return 1;
  }
}
