#ifndef ARGUMENT_HANDLER_HPP
#define ARGUMENT_HANDLER_HPP

#include "./cmd-line-args.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

// Constants
constexpr const char* DBUS_SERVICE_NAME = "org.mpris.MediaPlayer2.inLimbo";
constexpr const char* VERSION           = "2.3 (ALPHA)";

bool shouldRunApp = false;

enum class ConsoleColor
{
  Reset,
  Green,
  Blue,
  Yellow,
  Red,
  Cyan,
  Magenta
};

class ColorManager
{
public:
  static std::string_view getColor(ConsoleColor color)
  {
    static const std::unordered_map<ConsoleColor, std::string_view> colorCodes = {
      {ConsoleColor::Reset, "\033[0m"},   {ConsoleColor::Green, "\033[32m"},
      {ConsoleColor::Blue, "\033[34m"},   {ConsoleColor::Yellow, "\033[33m"},
      {ConsoleColor::Red, "\033[31m"},    {ConsoleColor::Cyan, "\033[36m"},
      {ConsoleColor::Magenta, "\033[35m"}};
    return colorCodes.at(color);
  }
};

class ArgumentHandler
{
public:
  struct Paths
  {
    std::string configPath;
    std::string libBinPath;
    std::string cacheDir;
  };

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
      {"--update-cache-run", [&]() { handleUpdateCacheRun(paths.libBinPath); }}};

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
  static void colorPrint(ConsoleColor color, std::string_view label, std::string_view value = "")
  {
    std::cout << ColorManager::getColor(color) << label
              << ColorManager::getColor(ConsoleColor::Reset) << value << "\n";
  }

  static void handleHelp(const CommandLineArgs& cmdArgs, const std::string& programName)
  {
    cmdArgs.printUsage(programName);
  }

  static void handleVersion() { colorPrint(ConsoleColor::Cyan, "inLimbo version: ", VERSION); }

  static void handleShowConfig(const std::string& configPath)
  {
    colorPrint(ConsoleColor::Green, "Config file location: ", configPath);
  }

  static void handleShowLogDir(const std::string& cacheDir)
  {
    colorPrint(ConsoleColor::Blue, "Log directory: ", cacheDir);
  }

  static void handleShowDBusName()
  {
    colorPrint(ConsoleColor::Magenta, "DBus name: ", DBUS_SERVICE_NAME);
  }

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

  static void handleUpdateCacheRun(const std::string& libBinPath)
  {
    std::cout << ColorManager::getColor(ConsoleColor::Blue) << "-- Updating Cache and running app..." << ColorManager::getColor(ConsoleColor::Reset) << std::endl;
    handleClearCache(libBinPath);
    shouldRunApp = true;
  }

};

#endif // ARGUMENT_HANDLER_HPP
