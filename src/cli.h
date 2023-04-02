
#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

inline std::string read_arg_val(std::vector<std::string>& args, std::vector<std::string>::iterator it)
{
  if (std::next(it) == args.end())
    throw std::runtime_error("missing value for cli option: " + *it);

  std::string val{ std::move(*std::next(it)) };
  args.erase(it, it + 2);
  return val;
}

inline std::string read_arg(std::vector<std::string>& args, const std::vector<std::string>& names)
{
  auto it = std::find_if(args.begin(), args.end(), [&names](const std::string& a) {
    return std::find(names.begin(), names.end(), a) != names.end();
    });

  if (it == args.end())
    throw std::runtime_error("missing cli option: " + names.front());

  return read_arg_val(args, it);
}

inline bool read_optional_flag(std::vector<std::string>& args, const std::vector<std::string>& names)
{
  auto it = std::find_if(args.begin(), args.end(), [&names](const std::string& a) {
    return std::find(names.begin(), names.end(), a) != names.end();
    });

  if (it == args.end())
    return false;

  args.erase(it);
  return true;
}
