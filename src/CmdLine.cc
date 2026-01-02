#include "CmdLine.hpp"
#include <sstream>

namespace cli
{

CmdLine::CmdLine(const std::string& program, const std::string& description)
    : m_program(program), m_options(program, description)
{
  addFlag("General", "help", 'h', "Show help");
}

void CmdLine::addFlag(const std::string& group, const std::string& longName, char shortName,
                      const std::string& description, Requirement requirement)
{
  auto& adder = groupAdder(group);
  adder(makeSpec(longName, shortName), description, cxxopts::value<bool>()->default_value("false"));

  m_knownOptions.push_back(longName);

  if (requirement == Requirement::Required)
    m_requiredOptions.push_back(longName);
}

void CmdLine::parse(int argc, char** argv)
{
  try
  {
    m_result = std::make_unique<cxxopts::ParseResult>(m_options.parse(argc, argv));
  }
  catch (const std::exception& ex)
  {
    throw CliError{formatError("Invalid command line", "argument parsing", ex.what())};
  }

  if (has("help"))
    throw HelpRequested{colorizeHelp(m_options.help())};

  for (const auto& opt : m_requiredOptions)
  {
    if (!has(opt))
    {
      throw CliError{formatError("Missing required option", "--" + opt,
                                 "this option must be explicitly provided")};
    }
  }

  for (auto& v : m_validators)
    v();
}

auto CmdLine::has(const std::string& longName) const -> bool
{
  ensureParsed();
  return m_result->count(longName) > 0;
}

auto CmdLine::help() const -> std::string { return m_options.help(); }

/* ================= PRIVATE HELPERS ================= */

auto CmdLine::useColor() -> bool { return isatty(fileno(stderr)); }

auto CmdLine::color(const std::string& c, const std::string& s) -> std::string
{
  if (!useColor())
    return s;
  return c + s + "\033[0m";
}

auto CmdLine::red(const std::string& s) -> std::string { return color("\033[31m", s); }

auto CmdLine::cyan(const std::string& s) -> std::string { return color("\033[36m", s); }

auto CmdLine::bold(const std::string& s) -> std::string { return color("\033[1m", s); }

auto CmdLine::formatError(const std::string& title, const std::string& option,
                          const std::string& reason) -> std::string
{
  std::string out;
  out += red("✗ ") + bold(title) + "\n\n";
  out += "  • Option : " + cyan(option) + "\n";
  out += "  • Reason : " + reason + "\n\n";
  out += "  Hint: Run with " + cyan("--help") + "\n";
  return out;
}

auto CmdLine::colorizeHelp(const std::string& help) -> std::string
{
  std::ostringstream out;
  std::istringstream in(help);
  std::string        line;
  bool               firstLine = true;

  while (std::getline(in, line))
  {
    if (firstLine && !line.empty())
    {
      out << "\n" << bold(color("\033[32m", line)) << "\n\n";
      firstLine = false;
      continue;
    }

    firstLine = false;

    if (line.ends_with("options:"))
    {
      out << bold(color("\033[36m", line)) << "\n";
      continue;
    }

    out << line << "\n";
  }

  return out.str();
}

auto CmdLine::makeSpec(const std::string& longName, char shortName) -> std::string
{
  if (shortName)
    return longName + "," + shortName;
  return longName;
}

auto CmdLine::groupAdder(const std::string& group) -> cxxopts::OptionAdder&
{
  auto it = m_groups.find(group);
  if (it != m_groups.end())
    return *it->second;

  auto adder = std::make_unique<cxxopts::OptionAdder>(m_options.add_options(group));

  auto& ref = *adder;
  m_groups.emplace(group, std::move(adder));
  return ref;
}

void CmdLine::ensureParsed() const
{
  if (!m_result)
  {
    throw CliError{formatError("Internal error", "command line", "parse() was not called")};
  }
}

} // namespace cli
