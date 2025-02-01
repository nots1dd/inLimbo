/**
 * @file argument_handler.hpp
 * @brief Handles command-line argument processing for the inLimbo TUI music player.
 *
 * This header defines the `ArgumentHandler` class, which provides functionality to
 * process command-line arguments and execute corresponding actions. It also includes
 * helper structures and utilities for managing console output with color.
 */
#pragma once

#include "./cmd-line-args.hpp"
#include <filesystem>
#include <iostream>
#include <string_view>
#include <unordered_map>

// Constants
constexpr const char* DBUS_SERVICE_NAME =
  "org.mpris.MediaPlayer2.inLimbo";                   ///< DBus service name used by inLimbo.
constexpr const char* VERSION        = "2.9 (ALPHA)"; ///< Current version of the application.
constexpr const char* REPOSITORY_URL = "https://github.com/nots1dd/inLimbo";

bool shouldRunApp =
  false; ///< Indicates if the application should proceed to run after handling arguments.
bool printSongTree = false;

/**
 * @enum ConsoleColor
 * @brief Represents ANSI console colors for colored output.
 */
enum class ConsoleColor
{
  Reset,  ///< Resets the console color to default.
  Green,  ///< Green color code.
  Blue,   ///< Blue color code.
  Yellow, ///< Yellow color code.
  Red,    ///< Red color code.
  Cyan,   ///< Cyan color code.
  Magenta ///< Magenta color code.
};

/**
 * @class ColorManager
 * @brief Provides utilities for managing console colors.
 *
 * The `ColorManager` class contains methods to retrieve ANSI color codes for
 * use in console output.
 */
class ColorManager
{
public:
  /**
   * @brief Retrieves the ANSI color code for a given `ConsoleColor`.
   *
   * @param color The `ConsoleColor` enum value.
   * @return A string view representing the ANSI color code.
   */
  static auto getColor(ConsoleColor color) -> std::string_view
  {
    static const std::unordered_map<ConsoleColor, std::string_view> colorCodes = {
      {ConsoleColor::Reset, "\033[0m"},   {ConsoleColor::Green, "\033[32m"},
      {ConsoleColor::Blue, "\033[34m"},   {ConsoleColor::Yellow, "\033[33m"},
      {ConsoleColor::Red, "\033[31m"},    {ConsoleColor::Cyan, "\033[36m"},
      {ConsoleColor::Magenta, "\033[35m"}};
    return colorCodes.at(color);
  }
};

/**
 * @class ArgumentHandler
 * @brief Handles processing of command-line arguments and executes corresponding actions.
 *
 * The `ArgumentHandler` class is responsible for parsing command-line arguments, executing
 * predefined actions for valid flags, and managing application paths. It also includes utilities
 * for colorful console output.
 */
class ArgumentHandler
{
public:
  /**
   * @struct Paths
   * @brief Represents essential application paths.
   *
   * The `Paths` structure contains paths to configuration files, binary libraries,
   * and cache directories used by the application.
   */
  struct Paths
  {
    std::string configPath; ///< Path to the configuration file.
    std::string libBinPath; ///< Path to the binary library.
    std::string cacheDir;   ///< Path to the cache directory.
  };

  /**
   * @brief Handles command-line arguments and executes corresponding actions.
   *
   * This method processes all valid flags provided through the `CommandLineArgs` object and
   * invokes the corresponding handler functions. For unsupported flags, an error message is
   * displayed.
   *
   * @param cmdArgs The `CommandLineArgs` object containing parsed command-line arguments.
   * @param programName The name of the program (typically `argv[0]`).
   * @param paths The application paths required for handling certain flags.
   */
  static void handleArguments(const CommandLineArgs& cmdArgs, const std::string& programName,
                              const Paths& paths)
  {
    const std::unordered_map<std::string, std::function<void()>> argumentHandlers = {
      {"--help", [&]() { handleHelp(cmdArgs, programName); }},
      {"--version", [&]() { handleVersion(); }},
      {"--clear-cache", [&]() { handleClearCache(paths.libBinPath); }},
      {"--show-config-file", [&]() { handleShowConfig(paths.configPath); }},
      {"--show-log-dir", [&]() { handleShowLogDir(paths.cacheDir); }},
      {"--show-dbus-name", [&]() { handleShowDBusName(); }},
      {"--update-cache-run", [&]() { handleUpdateCacheRun(paths.libBinPath); }},
      {"--print-song-tree", [&]() { handlePrintSongTree(); }}};

    for (const auto& [flag, handler] : argumentHandlers)
    {
      if (cmdArgs.hasFlag(flag))
      {
        handler();
        if (!shouldRunApp)
          exit(0);
      }
    }
  }

private:
  /**
   * @brief Prints a message with a specific console color.
   *
   * @param color The color to use for the label.
   * @param label The label to print in the specified color.
   * @param value (Optional) The value to print after the label.
   */
  static void colorPrint(ConsoleColor color, std::string_view label, std::string_view value = "")
  {
    std::cout << ColorManager::getColor(color) << label
              << ColorManager::getColor(ConsoleColor::Reset) << value << "\n";
  }

  /**
   * @brief Handles the `--help` flag by displaying usage information.
   *
   * @param cmdArgs The `CommandLineArgs` object containing parsed arguments.
   * @param programName The name of the program (typically `argv[0]`).
   */
  static void handleHelp(const CommandLineArgs& cmdArgs, const std::string& programName)
  {
    cmdArgs.printUsage(programName);
  }

  /**
   * @brief Handles the `--version` flag by displaying the application version.
   */
  static void handleVersion() { colorPrint(ConsoleColor::Cyan, "inLimbo version: ", VERSION); }

  /**
   * @brief Handles the `--show-config-file` flag by displaying the configuration file path.
   *
   * @param configPath The path to the configuration file.
   */
  static void handleShowConfig(const std::string& configPath)
  {
    colorPrint(ConsoleColor::Green, "Config file location: ", configPath);
  }

  /**
   * @brief Handles the `--show-log-dir` flag by displaying the log directory path.
   *
   * @param cacheDir The path to the cache directory.
   */
  static void handleShowLogDir(const std::string& cacheDir)
  {
    colorPrint(ConsoleColor::Blue, "Log directory: ", cacheDir);
  }

  /**
   * @brief Handles the `--show-dbus-name` flag by displaying the DBus service name.
   */
  static void handleShowDBusName()
  {
    colorPrint(ConsoleColor::Magenta, "DBus name: ", DBUS_SERVICE_NAME);
  }

  /**
   * @brief Handles the `--clear-cache` flag by clearing the cache directory.
   *
   * This function attempts to remove the cache file or directory specified by `libBinPath`.
   * If the path does not exist or an error occurs, appropriate messages are displayed.
   *
   * @param libBinPath The path to the cache file or directory.
   */
  static void handleClearCache(const std::string& libBinPath)
  {
    try
    {
      if (std::filesystem::exists(libBinPath))
      {
        std::filesystem::remove(libBinPath);
        colorPrint(ConsoleColor::Green, "Cache cleared successfully: ", libBinPath);
      }
      else
      {
        colorPrint(ConsoleColor::Yellow, "No cache file found to clear: ", libBinPath);
      }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
      colorPrint(ConsoleColor::Red, "Error clearing cache: ", e.what());
    }
  }

  /**
   * @brief Handles the `--update-cache-run` flag by clearing the cache and setting the app to run.
   *
   * This function clears the cache and sets the `shouldRunApp` flag to `true`, indicating
   * that the application should proceed to execution.
   *
   * @param libBinPath The path to the cache file or directory.
   */
  static void handleUpdateCacheRun(const std::string& libBinPath)
  {
    std::cout << ColorManager::getColor(ConsoleColor::Blue)
              << "-- Updating Cache and running app..."
              << ColorManager::getColor(ConsoleColor::Reset) << std::endl;
    handleClearCache(libBinPath);
    shouldRunApp = true;
  }
  
  static void handlePrintSongTree()
  {
    printSongTree = true;
    shouldRunApp = true;
  }
};
