#ifndef COMMAND_LINE_ARGS_HPP
#define COMMAND_LINE_ARGS_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

class CommandLineArgs
{
private:
    std::map<std::string, std::string> args;   // flag-value pairs
    std::vector<std::string> positionalArgs;  // positional arguments

public:
    static const std::vector<std::string> validFlags; // Valid flags

    CommandLineArgs(int argc, char* argv[])
    {
        parseArgs(argc, argv);
    }

    std::string get(const std::string& flag, const std::string& defaultValue = "") const
    {
        auto it = args.find(flag);
        if (it != args.end())
        {
            return it->second;
        }
        return defaultValue;
    }

    bool hasFlag(const std::string& flag) const
    {
        return args.find(flag) != args.end();
    }

    const std::vector<std::string>& getPositionalArgs() const
    {
        return positionalArgs;
    }

    void printUsage(const std::string& programName) const
    {
        std::cerr << "Music player that keeps you in Limbo.\n";
        std::cerr << "Usage: " << programName << " [options] [positional arguments]\n\n";
        if (!validFlags.empty())
        {
            std::cerr << "Valid options:\n";
            for (const auto& flag : validFlags)
            {
                std::cerr << "  " << flag << "\n";
            }
        }
    }

private:
    void parseArgs(int argc, char* argv[])
    {
        for (int i = 1; i < argc; ++i) // Skip argv[0] (program name)
        {
            std::string arg = argv[i];

            if (arg[0] == '-') // It's a flag
            {
                std::string flag = arg;
                std::string value;

                // Check if the flag has a value
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    value = argv[++i]; // Consume the next argument as the value
                }

                // Validate the flag
                if (std::find(validFlags.begin(), validFlags.end(), flag) == validFlags.end())
                {
                    throw std::invalid_argument("Invalid flag: " + flag);
                }

                args[flag] = value;
            }
            else // It's a positional argument
            {
                positionalArgs.push_back(arg);
            }
        }
    }
};

// Define valid flags globally
const std::vector<std::string> CommandLineArgs::validFlags = {
    "--help", "--show-dbus-name", "--version", "--clear-cache", 
    "--show-config-file", "--show-log-dir"
};

#endif // COMMAND_LINE_ARGS_HPP
