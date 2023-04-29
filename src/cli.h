
#pragma once

/**
 * \file cli.h
 * \brief provides command-line parsing helper functions
 * 
 * This file provides several helper functions to help parse the 
 * command line arguments.
 */

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

/**
 * \brief read a command line argument
 * \param args   list of command line arguments
 * \param names  possible names for the argument
 * \return the value of the argument
 * 
 * If the argument cannot be found under any of the provided \a names, an exception 
 * of type std::runtime_error is thrown.
 * 
 * The argument is removed from \a args after being read.
 */
inline std::string read_arg(std::vector<std::string>& args, const std::vector<std::string>& names)
{
  auto it = std::find_if(args.begin(), args.end(), [&names](const std::string& a) {
    return std::find(names.begin(), names.end(), a) != names.end();
    });

  if (it == args.end())
    throw std::runtime_error("missing cli option: " + names.front());

  return read_arg_val(args, it);
}

/**
 * \brief look for an optional flag in the command line arguments
 * \param args   the list of command line arguments
 * \param names  possible names for the flag to look for
 * \return true if the flag was found, false otherwise
 * 
 * If the flag is found in \a args, it is removed from the list.
 */
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
