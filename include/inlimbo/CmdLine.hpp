#pragma once

#include <cxxopts.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <functional>
#include <type_traits>
#include <vector>
#include <unistd.h>

namespace cli {

enum class Requirement {
    Optional,
    Required
};

class CmdLine {
public:
    struct HelpRequested {
        std::string text;
    };

    struct CliError {
        std::string message;
    };

    CmdLine(const std::string& program,
            const std::string& description = "")
        : m_program(program),
          m_options(program, description)
    {
        addFlag("General", "help", 'h', "Show help");
    }

    template<typename T>
    void add(const std::string& group,
             const std::string& longName,
             char shortName,
             const std::string& description,
             std::optional<T> defaultValue = std::nullopt,
             std::function<bool(const T&)> validator = nullptr,
             std::string hint = {},
             Requirement requirement = Requirement::Optional)
    {
        auto& adder = groupAdder(group);
        const std::string spec = makeSpec(longName, shortName);

        if (defaultValue) {
            adder(spec, description,
                  cxxopts::value<T>()->default_value(toString(*defaultValue)));
        } else {
            adder(spec, description, cxxopts::value<T>());
        }

        m_knownOptions.push_back(longName);

        if (requirement == Requirement::Required)
            m_requiredOptions.push_back(longName);

        if (validator) {
            m_validators.emplace_back(
                [this, longName, validator, hint]() -> auto {
                    if (!has(longName)) return;

                    const T& value = (*m_result)[longName].as<T>();
                    if (!validator(value)) {
                        throw CliError{
                            formatError(
                                "Invalid value",
                                "--" + longName,
                                hint.empty() ? "value not accepted" : hint
                            )
                        };
                    }
                }
            );
        }
    }

    void addFlag(const std::string& group,
                 const std::string& longName,
                 char shortName,
                 const std::string& description,
                 Requirement requirement = Requirement::Optional)
    {
        auto& adder = groupAdder(group);
        adder(makeSpec(longName, shortName),
              description,
              cxxopts::value<bool>()->default_value("false"));

        m_knownOptions.push_back(longName);

        if (requirement == Requirement::Required)
            m_requiredOptions.push_back(longName);
    }

    void parse(int argc, char** argv) {
        try {
            m_result = std::make_unique<cxxopts::ParseResult>(
                m_options.parse(argc, argv));
        }
        catch (const std::exception& ex) {
            throw CliError{
                formatError(
                    "Invalid command line",
                    "argument parsing",
                    ex.what()
                )
            };
        }

        if (has("help"))
            throw HelpRequested{ m_options.help() };

        for (const auto& opt : m_requiredOptions) {
            if (!has(opt)) {
                throw CliError{
                    formatError(
                        "Missing required option",
                        "--" + opt,
                        "this option must be explicitly provided"
                    )
                };
            }
        }

        for (auto& v : m_validators)
            v();
    }

    [[nodiscard]] auto has(const std::string& longName) const -> bool {
        ensureParsed();
        return m_result->count(longName) > 0;
    }

    template<typename T>
    [[nodiscard]] auto get(const std::string& longName) const -> T {
        ensureParsed();
        if (!has(longName)) {
            throw CliError{
                formatError(
                    "Missing required option",
                    "--" + longName,
                    "run with --help to see available options"
                )
            };
        }
        return (*m_result)[longName].as<T>();
    }

    template<typename T>
    [[nodiscard]] auto getOptional(const std::string& longName) const -> std::optional<T> {
        ensureParsed();
        if (!has(longName))
            return std::nullopt;
        return (*m_result)[longName].as<T>();
    }

    [[nodiscard]] auto help() const -> std::string {
        return m_options.help();
    }

private:
    std::string m_program;
    cxxopts::Options m_options;
    std::unique_ptr<cxxopts::ParseResult> m_result;

    std::unordered_map<std::string,
        std::unique_ptr<cxxopts::OptionAdder>> m_groups;

    std::vector<std::function<void()>> m_validators;
    std::vector<std::string> m_knownOptions;
    std::vector<std::string> m_requiredOptions;

    static auto useColor() -> bool {
        return isatty(fileno(stderr));
    }

    static auto color(const std::string& c, const std::string& s) -> std::string {
        if (!useColor()) return s;
        return c + s + "\033[0m";
    }

    static auto red(const std::string& s)  -> std::string { return color("\033[31m", s); }
    static auto cyan(const std::string& s) -> std::string { return color("\033[36m", s); }
    static auto bold(const std::string& s) -> std::string { return color("\033[1m", s); }

    static auto formatError(const std::string& title,
                            const std::string& option,
                            const std::string& reason) -> std::string
    {
        std::string out;
        out += red("✗ ") + bold(title) + "\n\n";
        out += "  • Option : " + cyan(option) + "\n";
        out += "  • Reason : " + reason + "\n\n";
        out += "  Hint: Run with " + cyan("--help") + "\n";
        return out;
    }

    static auto makeSpec(const std::string& longName, char shortName) -> std::string {
        if (shortName)
            return longName + "," + shortName;
        return longName;
    }

    auto groupAdder(const std::string& group) -> cxxopts::OptionAdder& {
        auto it = m_groups.find(group);
        if (it != m_groups.end())
            return *it->second;

        auto adder = std::make_unique<cxxopts::OptionAdder>(
            m_options.add_options(group));
        auto& ref = *adder;
        m_groups.emplace(group, std::move(adder));
        return ref;
    }

    void ensureParsed() const {
        if (!m_result) {
            throw CliError{
                formatError(
                    "Internal error",
                    "command line",
                    "parse() was not called"
                )
            };
        }
    }

    template<typename T>
    static auto toString(const T& v) -> std::string {
        if constexpr (std::is_same_v<T, std::string>)
            return v;
        else if constexpr (std::is_same_v<T, bool>)
            return v ? "true" : "false";
        else
            return std::to_string(v);
    }
};

} // namespace cli
