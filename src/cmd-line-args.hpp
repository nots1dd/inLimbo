#ifndef COMMAND_LINE_ARGS_HPP
#define COMMAND_LINE_ARGS_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class CommandLineArgs
{
private:
  std::map<std::string, std::string> args;
  std::vector<std::string>           positionalArgs;
  bool                               hasError = false;
  std::string                        errorMessage;

public:
  static const std::vector<std::string> validFlags;

  CommandLineArgs(int argc, char* argv[])
  {
    parseArgs(argc, argv);
    if (hasError)
    {
      std::cerr << "\033[31mError: " << errorMessage << "\033[0m\n\n";
      printUsage(argv[0]);
      exit(1);
    }
  }

  std::string get(const std::string& flag, const std::string& defaultValue = "") const
  {
    auto it = args.find(flag);
    return (it != args.end()) ? it->second : defaultValue;
  }

  bool hasFlag(const std::string& flag) const { return args.find(flag) != args.end(); }

  const std::vector<std::string>& getPositionalArgs() const { return positionalArgs; }

  void printUsage(const std::string& programName) const
  {
    std::cout << "\033[1mMusic player that keeps you in Limbo.\033[0m\n";
    std::cout << "\033[1mUsage:\033[0m " << programName << " [options] [positional arguments]\n\n";

    if (!validFlags.empty())
    {
      std::cout << "\033[1mValid options:\033[0m\n";
      for (const auto& flag : validFlags)
      {
        std::cout << "  " << flag << "\n";
      }
    }

    std::cout << "\n";

    // Project details and additional information
    std::cout << "\033[1mProject Information:\033[0m\n";
    std::cout << "-  Name: inLimbo - Terminal-based Music Player\n";
    std::cout << "-  Author: Siddharth Karnam (nots1dd)\n";
    std::cout << "-  License: GNU GPL v3\n";
    std::cout << "-  Website: https://nots1dd.github.io/inLimbo/ (has Doxygen documentation)\n";
    std::cout << "For any issues visit <https://github.com/nots1dd/inLimbo>\n";
    std::cout << std::endl;

    std::cout << "\033[1mDescription:\033[0m\n";
    std::cout << "  inLimbo is a TUI music player that supports seamless playback and efficient metadata handling.\n  Designed for minimalism and ease of use.\n\n\033[1mKeeps YOU in Limbo...\033[0m";
    std::cout << "\n";
  }

private:
  void parseArgs(int argc, char* argv[])
  {
    for (int i = 1; i < argc; ++i)
    {
      std::string arg = argv[i];

      if (arg[0] == '-')
      {
        std::string flag = arg;
        std::string value;

        if (i + 1 < argc && argv[i + 1][0] != '-')
        {
          value = argv[++i];
        }

        // Find closest matching flag for error message
        if (std::find(validFlags.begin(), validFlags.end(), flag) == validFlags.end())
        {
          hasError = true;

          // Find the closest matching flag for a helpful suggestion
          std::string closestMatch = findClosestMatch(flag);
          if (!closestMatch.empty())
          {
            errorMessage = "Invalid flag: '" + flag + "'. Did you mean '" + closestMatch + "'?";
          }
          else
          {
            errorMessage = "Invalid flag: '" + flag + "'";
          }
          return;
        }

        args[flag] = value;
      }
      else
      {
        positionalArgs.push_back(arg);
      }
    }
  }

  // Helper function to find the closest matching valid flag
  std::string findClosestMatch(const std::string& invalidFlag) const
  {
    std::string bestMatch;
    size_t      minDiff = std::string::npos;

    for (const auto& validFlag : validFlags)
    {
      // Simple distance calculation - how many characters are different
      size_t diff   = 0;
      size_t minLen = std::min(invalidFlag.length(), validFlag.length());

      for (size_t i = 0; i < minLen; ++i)
      {
        if (invalidFlag[i] != validFlag[i])
          diff++;
      }
      diff += std::abs(int(invalidFlag.length() - validFlag.length()));

      if (diff < minDiff)
      {
        minDiff   = diff;
        bestMatch = validFlag;
      }
    }

    // Only suggest if the match is reasonably close
    return (minDiff <= 3) ? bestMatch : "";
  }
};

// Define valid flags globally
const std::vector<std::string> CommandLineArgs::validFlags = {
  "--help",        "--show-dbus-name",   "--version",
  "--clear-cache", "--show-config-file", "--show-log-dir", "--update-cache-run"};

#endif // COMMAND_LINE_ARGS_HPP
