
#include <iostream>
#include <string>
#include <vector>

extern void scan(std::vector<std::string> args);
extern void export_(std::vector<std::string> args);

[[noreturn]] void help()
{
  std::cout << "csnap is a libclang-based command-line utility to create snapshots of C++ programs." << std::endl;
  std::cout << std::endl;
  std::cout << "Syntax:" << std::endl;
  std::cout << "  csnap scan --sln <Visual Studio solution> --ouput <snapshot.db>" << std::endl;
  std::cout << "  csnap export -i <snapshot.db> --ouput <outdir>" << std::endl;

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
  else  if (args.at(1) == "export")
  {
    args.erase(args.begin(), args.begin() + 2);
    export_(args);
  }
  else
  {
    std::cerr << "unrecognized command " << args.at(1) << std::endl;
    return 1;
  }
}
