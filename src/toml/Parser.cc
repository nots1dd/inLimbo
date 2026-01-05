#include "toml/Parser.hpp"
#include "Logger.hpp"
#include "utils/PathResolve.hpp"
#include <filesystem>

namespace tomlparser
{

std::optional<toml::parse_result> Config::s_config;

void Config::load()
{
  const auto path = utils::getConfigPathWithFile("config.toml");

  if (!std::filesystem::exists(path.c_str()))
  {
    LOG_ERROR("config.toml not found at '{}'", path);
    throw std::runtime_error("Missing config.toml");
  }

  LOG_DEBUG("Loading config.toml: {}", path);
  s_config = toml::parse_file(path);
}

void Config::loadFrom(const std::string& path)
{
  if (!std::filesystem::exists(path))
  {
    LOG_ERROR("Custom config not found at '{}'", path);
    throw std::runtime_error("Missing custom config");
  }

  LOG_DEBUG("Loading custom config: {}", path);
  s_config = toml::parse_file(path);
}

auto Config::isLoaded() noexcept -> bool { return s_config.has_value(); }

// ------------------------------------------------------------
// Optional accessors
// ------------------------------------------------------------

auto Config::getString(std::string_view section, std::string_view key, std::string_view fallback)
  -> std::string_view
{
  ensureLoaded();
  return (*s_config)[section][key].value_or(fallback);
}

auto Config::getInt(std::string_view section, std::string_view key, int64_t fallback) -> int64_t
{
  ensureLoaded();
  return (*s_config)[section][key].value_or(fallback);
}

auto Config::getBool(std::string_view section, std::string_view key, bool fallback) -> bool
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<bool>())
    return *v;

  return fallback;
}

// ------------------------------------------------------------
// Required accessors
// ------------------------------------------------------------

auto Config::requireString(std::string_view section, std::string_view key) -> std::string
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<std::string>())
    return *v;

  throwMissing(section, key);

  return "";
}

auto Config::requireInt(std::string_view section, std::string_view key) -> int64_t
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<int64_t>())
    return *v;

  throwMissing(section, key);

  return false;
}

auto Config::requireBool(std::string_view section, std::string_view key) -> bool
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<bool>())
    return *v;

  throwMissing(section, key);

  return false;
}

// ------------------------------------------------------------
// Table access
// ------------------------------------------------------------

auto Config::table(std::string_view section) -> const toml::table&
{
  ensureLoaded();

  const auto* tbl = (*s_config)[section].as_table();
  if (!tbl)
    throw std::runtime_error("TOML section is not a table");

  return *tbl;
}

// ------------------------------------------------------------
// Internals
// ------------------------------------------------------------

void Config::ensureLoaded()
{
  if (!s_config)
    throw std::runtime_error("Config accessed before load()");
}

void Config::throwMissing(std::string_view section, std::string_view key)
{
  LOG_ERROR("Missing or invalid config value: [{}].{}", section, key);
  throw std::runtime_error("Invalid configuration");
}

} // namespace tomlparser
