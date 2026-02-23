#include "config/Config.hpp"
#include "Logger.hpp"
#include "utils/fs/Paths.hpp"
#include <filesystem>

namespace config
{

std::optional<::toml::parse_result> Config::s_config;

void Config::load()
{
  const auto path = utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME);

  if (!std::filesystem::exists(path.c_str()))
  {
    LOG_ERROR("config.toml not found at '{}'", path.c_str());
    throw std::runtime_error("config::Config: Missing config.toml");
  }

  LOG_DEBUG("Loading config.toml: {}", path.c_str());
  s_config = ::toml::parse_file(path.c_str());
}

void Config::loadFrom(const Path& path)
{
  if (!std::filesystem::exists(path.c_str()))
  {
    LOG_ERROR("Custom config not found at '{}'", path);
    throw std::runtime_error("config::Config: Missing custom config file!");
  }

  LOG_DEBUG("Loading custom config: {}", path);
  s_config = ::toml::parse_file(path);
}

auto Config::isLoaded() noexcept -> bool { return s_config.has_value(); }

// ------------------------------------------------------------
// Optional accessors
// ------------------------------------------------------------

auto Config::getString(toml::SectionView section, toml::KeyView key, std::string_view fallback)
  -> std::string_view
{
  ensureLoaded();
  return (*s_config)[section][key].value_or(fallback);
}

auto Config::getInt(toml::SectionView section, toml::KeyView key, int64_t fallback) -> int64_t
{
  ensureLoaded();
  return (*s_config)[section][key].value_or(fallback);
}

auto Config::getBool(toml::SectionView section, toml::KeyView key, bool fallback) -> bool
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<bool>())
    return *v;

  return fallback;
}

// ------------------------------------------------------------
// Required accessors
// ------------------------------------------------------------

auto Config::requireString(toml::SectionView section, toml::KeyView key) -> std::string
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<std::string>())
    return *v;

  throwMissing(section, key);

  return "";
}

auto Config::requireInt(toml::SectionView section, toml::KeyView key) -> i64
{
  ensureLoaded();

  if (auto v = (*s_config)[section][key].value<i64>())
    return *v;

  throwMissing(section, key);

  return false;
}

auto Config::requireBool(toml::SectionView section, toml::KeyView key) -> bool
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

auto Config::table(toml::SectionView section) -> const ::toml::table&
{
  ensureLoaded();

  const auto* tbl = (*s_config)[section].as_table();
  if (!tbl)
    throw std::runtime_error("config::Config: TOML section is not a table!");

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

void Config::throwMissing(toml::SectionView section, toml::KeyView key)
{
  LOG_ERROR("Missing or invalid config value: [{}].{}", section, key);
  throw std::runtime_error("config::Config: Invalid configuration!");
}

} // namespace config
