/**
 * @file command_line_args.hpp
 * @brief Provides a class for parsing and handling command-line arguments for the inLimbo TUI music
 * player.
 *
 * This header defines the `CommandLineArgs` class, which simplifies the parsing and management
 * of command-line arguments. It validates flags, extracts positional arguments, and provides
 * helpful error messages for invalid flags. Additionally, it offers usage information and project
 * details.
 */
#pragma once

#include "./helpers/levenshtein.hpp"
#include <iostream>
#include <map>

/**
 * @class CommandLineArgs
 * @brief Parses and validates command-line arguments.
 *
 * The `CommandLineArgs` class handles parsing flags, extracting positional arguments,
 * and providing detailed error messages for invalid flags. It is designed for the
 * inLimbo TUI music player and includes project-specific details in its usage output.
 */
class CommandLineArgs
{
private:
  std::map<std::string, std::string> args; ///< Stores flags and their corresponding values.
  std::vector<std::string>           positionalArgs; ///< Stores non-flag, positional arguments.
  bool        hasError = false; ///< Indicates if an error occurred during parsing.
  std::string errorMessage;     ///< Stores the error message if parsing fails.

public:
  /**
   * @brief List of valid flags supported by the application.
   */
  static const std::vector<std::string> validFlags;
  /**
   * @brief Constructs a CommandLineArgs object and parses command-line arguments.
   *
   * If parsing fails, an error message is displayed, usage information is printed,
   * and the program exits with an error code.
   *
   * @param argc The number of command-line arguments.
   * @param argv The array of command-line arguments.
   */
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
  /**
   * @brief Retrieves the value associated with a flag.
   *
   * If the flag is not present, a default value is returned.
   *
   * @param flag The flag to look up.
   * @param defaultValue The value to return if the flag is not found (default is an empty string).
   * @return The value associated with the flag or the default value if the flag is not found.
   */
  [[nodiscard]]
  auto get(const std::string& flag, const std::string& defaultValue = "") const -> std::string
  {
    auto it = args.find(flag);
    return (it != args.end()) ? it->second : defaultValue;
  }

  /**
   * @brief Checks if a specific flag was provided.
   *
   * @param flag The flag to check.
   * @return `true` if the flag is present, otherwise `false`.
   */
  [[nodiscard]]
  auto hasFlag(const std::string& flag) const -> bool
  {
    return args.find(flag) != args.end();
  }
  /**
   * @brief Retrieves the list of positional arguments.
   *
   * @return A reference to a vector containing the positional arguments.
   */
  [[nodiscard]]
  auto getPositionalArgs() const -> const std::vector<std::string>&
  {
    return positionalArgs;
  }

  /**
   * @brief Prints usage information and program details.
   *
   * The output includes a description of the program, valid options,
   * and helpful links for project documentation and support.
   *
   * @param programName The name of the program (typically `argv[0]`).
   */
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
    std::cout << "  inLimbo is a TUI music player that supports seamless playback and efficient "
                 "metadata handling.\n  Designed for minimalism and ease of use.\n\n\033[1mKeeps "
                 "YOU in Limbo...\033[0m";
    std::cout << "\n";
  }

private:
  /**
   * @brief Parses the command-line arguments.
   *
   * This method extracts flags and their values, validates them, and
   * identifies positional arguments. If an invalid flag is encountered,
   * an error message is prepared.
   *
   * @param argc The number of command-line arguments.
   * @param argv The array of command-line arguments.
   */
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

  /**
   * @brief Finds the closest valid flag to a given invalid flag using Levenshtein distance.
   *
   * This method suggests a valid flag that is most similar to the invalid flag provided,
   * helping the user identify potential typos or errors.
   *
   * @param invalidFlag The invalid flag entered by the user.
   * @return The closest matching valid flag if it exists and is reasonably close; otherwise, an
   * empty string.
   */
  [[nodiscard]]
  auto findClosestMatch(const std::string& invalidFlag) const -> std::string
  {
    std::string bestMatch;
    size_t      minDist = std::string::npos;

    for (const auto& validFlag : validFlags)
    {
      // Compute Levenshtein distance
      size_t dist = levenshteinDistance(invalidFlag, validFlag);

      // Track the flag with the smallest distance
      if (dist < minDist)
      {
        minDist   = dist;
        bestMatch = validFlag;
      }
    }

    // Only suggest if the match is reasonably close (for example, max 3 edits away)
    return (minDist <= 3) ? bestMatch : "";
  }
};

// Define valid flags globally
const std::vector<std::string> CommandLineArgs::validFlags = {
  "--help",         "--show-dbus-name",  "--version", "--clear-cache", "--show-config-file",
  "--show-log-dir", "--update-cache-run"};
