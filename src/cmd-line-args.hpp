#ifndef COMMAND_LINE_ARGS_HPP
#define COMMAND_LINE_ARGS_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

class CommandLineArgs
{
private:
    std::map<std::string, std::string> args;   // flag-value pairs
    std::vector<std::string> positionalArgs;  // pos args
    std::vector<std::string> validFlags;      // Valid flags for error checking

public:
    CommandLineArgs(int argc, char* argv[], const std::vector<std::string>& allowedFlags = {})
        : validFlags(allowedFlags)
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
        std::cerr << "Usage: " << programName << " [options] [positional arguments]\n";
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

                // Validate the flag if a list of valid flags is provided
                if (!validFlags.empty() && std::find(validFlags.begin(), validFlags.end(), flag) == validFlags.end())
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

#endif // COMMAND_LINE_ARGS_HPP
