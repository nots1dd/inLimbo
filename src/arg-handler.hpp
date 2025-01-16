#ifndef ARGUMENT_HANDLER_HPP
#define ARGUMENT_HANDLER_HPP

#include "./cmd-line-args.hpp"
#include <iostream>
#include <stdexcept>
#include <filesystem>

#define DBUS_SERVICE_NAME "org.mpris.MediaPlayer2.inLimbo"

// ANSI color codes for output
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RED "\033[31m"
#define COLOR_CYAN "\033[36m"
#define COLOR_MAGENTA "\033[35m"

class ArgumentHandler
{
public:
    static void handleArguments(const CommandLineArgs& cmdArgs, const std::string& programName, const std::string& configPath, const std::string& libBinPath, const std::string& cacheDir)
    {
        if (cmdArgs.hasFlag("--help"))
        {
            printUsage(cmdArgs, programName);
            exit(0);
        }

        if (cmdArgs.hasFlag("--version"))
        {
            std::cout << COLOR_CYAN << "inLimbo version: ALPHA 2.0" << COLOR_RESET << "\n";
            exit(0);
        }

        if (cmdArgs.hasFlag("--clear-cache"))
        {
            clearCache(libBinPath);
            exit(0);
        }

        if (cmdArgs.hasFlag("--show-config-file"))
        {
            std::cout << COLOR_GREEN << "Config file location: " << COLOR_RESET << configPath << "\n";
            exit(0);
        }

        if (cmdArgs.hasFlag("--show-log-dir"))
        {
            std::cout << COLOR_BLUE << "Log directory: " << COLOR_RESET << cacheDir << "\n";
            exit(0);
        }

        if (cmdArgs.hasFlag("--show-dbus-name"))
        {
            std::cout << COLOR_MAGENTA << "DBus name: " << COLOR_RESET << DBUS_SERVICE_NAME << "\n";
            exit(0);
        }
    }

private:
    static void printUsage(const CommandLineArgs& cmdArgs, const std::string& programName)
    {
        cmdArgs.printUsage(programName);
    }

    static void clearCache(const std::string& libBinPath)
    {
        try
        {
            if (std::filesystem::exists(libBinPath))
            {
                std::filesystem::remove(libBinPath);
                std::cout << COLOR_GREEN << "Cache cleared successfully: " << COLOR_RESET << libBinPath << "\n";
            }
            else
            {
                std::cout << COLOR_YELLOW << "No cache file found to clear: " << COLOR_RESET << libBinPath << "\n";
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << COLOR_RED << "Error clearing cache: " << COLOR_RESET << e.what() << "\n";
        }
    }
};

#endif // ARGUMENT_HANDLER_HPP
