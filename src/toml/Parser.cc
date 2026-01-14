#include "toml/Parser.hpp"
#include "Logger.hpp"
#include "utils/PathResolve.hpp"
#include <filesystem>

namespace tomlparser
{

std::optional<toml::parse_result> Config::s_config;

void Config::load()
{
  const auto path = utils::getAppConfigPathWithFile("config.toml");

  if (!std::filesystem::exists(path.c_str()))
  {
    LOG_ERROR("config.toml not found at '{}'", path.c_str());
    throw std::runtime_error("Missing config.toml");
  }

  LOG_DEBUG("Loading config.toml: {}", path.c_str());
  s_config = toml::parse_file(path.c_str());
}

void Config::loadFrom(const Path& path)
{
  if (!std::filesystem::exists(path.c_str()))
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

auto Config::getString(SectionView section, KeyView key, std::string_view fallback)
  -> std::string_view
{
  ensureLoaded();
  return (*s_config)[section][key].value_or(fallback);
}

auto Config::getInt(SectionView section, KeyView key, int64_t fallback) -> int64_t
{
  ensureLoaded();
  return (*s_config)[section][key].value_or(fallback);
}

auto Config::getBool(SectionView section, KeyView key, bool fallback) -> bool
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<bool>())
    return *v;

  return fallback;
}

// ------------------------------------------------------------
// Required accessors
// ------------------------------------------------------------

auto Config::requireString(SectionView section, KeyView key) -> std::string
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<std::string>())
    return *v;

  throwMissing(section, key);

  return "";
}

auto Config::requireInt(SectionView section, KeyView key) -> int64_t
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<int64_t>())
    return *v;

  throwMissing(section, key);

  return false;
}

auto Config::requireBool(SectionView section, KeyView key) -> bool
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

auto Config::table(SectionView section) -> const toml::table&
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

void Config::throwMissing(SectionView section, KeyView key)
{
  LOG_ERROR("Missing or invalid config value: [{}].{}", section, key);
  throw std::runtime_error("Invalid configuration");
}

} // namespace tomlparser
