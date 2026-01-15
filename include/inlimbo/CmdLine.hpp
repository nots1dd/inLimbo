#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <cxxopts.hpp>
#include <unistd.h>

namespace cli
{

enum class Requirement
{
  Optional,
  Required
};

class CmdLine
{
public:
  struct HelpRequested
  {
    std::string text;
  };

  struct VersionRequested
  {
    std::string text;
  };

  struct CliError
  {
    std::string message;
  };

  CmdLine(const std::string& program, const std::string& description = "");

  template <typename T>
  void add(const std::string& group, const std::string& longName, char shortName,
           const std::string& description, std::optional<T> defaultValue = std::nullopt,
           std::function<bool(const T&)> validator = nullptr, std::string hint = {},
           Requirement requirement = Requirement::Optional);

  void addFlag(const std::string& group, const std::string& longName, char shortName,
               const std::string& description, Requirement requirement = Requirement::Optional);

  void parse(int argc, char** argv);

  [[nodiscard]] auto has(const std::string& longName) const -> bool;

  template <typename T> [[nodiscard]] auto get(const std::string& longName) const -> T;

  template <typename T>
  [[nodiscard]] auto getOptional(const std::string& longName) const -> std::optional<T>;

  [[nodiscard]] auto help() const -> std::string;

private:
  std::string                           m_program;
  cxxopts::Options                      m_options;
  std::unique_ptr<cxxopts::ParseResult> m_result;

  std::unordered_map<std::string, std::unique_ptr<cxxopts::OptionAdder>> m_groups;
  std::vector<std::function<void()>>                                     m_validators;
  std::vector<std::string>                                               m_knownOptions;
  std::vector<std::string>                                               m_requiredOptions;

  static auto useColor() -> bool;
  static auto color(const std::string& c, const std::string& s) -> std::string;
  static auto red(const std::string& s) -> std::string;
  static auto cyan(const std::string& s) -> std::string;
  static auto bold(const std::string& s) -> std::string;

  static auto formatError(const std::string& title, const std::string& option,
                          const std::string& reason) -> std::string;

  static auto colorizeHelp(const std::string& help) -> std::string;
  static auto makeSpec(const std::string& longName, char shortName) -> std::string;

  auto groupAdder(const std::string& group) -> cxxopts::OptionAdder&;
  void ensureParsed() const;

  template <typename T> static auto toString(const T& v) -> std::string;
};

/* ================= TEMPLATE IMPLEMENTATIONS ================= */

template <typename T>
void CmdLine::add(const std::string& group, const std::string& longName, char shortName,
                  const std::string& description, std::optional<T> defaultValue,
                  std::function<bool(const T&)> validator, std::string hint,
                  Requirement requirement)
{
  auto&             adder = groupAdder(group);
  const std::string spec  = makeSpec(longName, shortName);

  if (defaultValue)
    adder(spec, description, cxxopts::value<T>()->default_value(toString(*defaultValue)));
  else
    adder(spec, description, cxxopts::value<T>());

  m_knownOptions.push_back(longName);

  if (requirement == Requirement::Required)
    m_requiredOptions.push_back(longName);

  if (validator)
  {
    m_validators.emplace_back(
      [this, longName, validator, hint]() -> auto
      {
        if (!has(longName))
          return;

        const T& value = (*m_result)[longName].as<T>();
        if (!validator(value))
        {
          throw CliError{formatError("Invalid value", "--" + longName,
                                     hint.empty() ? "value not accepted" : hint)};
        }
      });
  }
}

template <typename T> auto CmdLine::get(const std::string& longName) const -> T
{
  ensureParsed();
  if (!has(longName))
  {
    throw CliError{formatError("Missing required option", "--" + longName,
                               "run with --help to see available options")};
  }
  return (*m_result)[longName].as<T>();
}

template <typename T>
auto CmdLine::getOptional(const std::string& longName) const -> std::optional<T>
{
  static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, bool> ||
                  std::is_arithmetic_v<T>,
                "CmdLine::getOptional<T>: unsupported type for CLI parsing");

  ensureParsed();
  if (!has(longName))
    return std::nullopt;
  return (*m_result)[longName].as<T>();
}

template <typename T> auto CmdLine::toString(const T& v) -> std::string
{
  if constexpr (std::is_same_v<T, std::string>)
    return v;
  else if constexpr (std::is_same_v<T, bool>)
    return v ? "true" : "false";
  else
    return std::to_string(v);
}

} // namespace cli
