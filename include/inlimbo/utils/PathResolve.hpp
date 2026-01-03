#pragma once

#include "Config.hpp"
#include "Env-Vars.hpp"
#include <stdexcept>
#include <string>

namespace utils
{

FORCE_INLINE auto getHomeDir() -> const std::string
{
  const char* homeDir = getenv("HOME");
  if (!homeDir)
  {
    throw std::runtime_error("HOME environment variable not found! Exiting program...");
  }

  return std::string(homeDir);
}

/**
 * @brief Retrieves the path to the configuration directory.
 *
 * By default, this function uses the HOME environment variable to determine the user's home
 * directory. However, it checks for the presence of a `INLIMBO_CONFIG_HOME` environment variable
 * first, which can override the default location. This allows for custom configuration paths during
 * testing.
 *
 * @return A string representing the base path to the configuration directory.
 * @throws std::runtime_error If the HOME environment variable is not found and no custom path is
 *         provided.
 */
FORCE_INLINE auto getBaseConfigPath() -> std::string
{
  const char* customConfigHome = getenv(INLIMBO_CUSTOM_CONFIG_ENV);
  if (customConfigHome)
  {
    return {customConfigHome};
  }

  return getHomeDir() + "/.config/inLimbo/";
}

/**
 * @brief Retrieves the full path to the configuration file.
 *
 * This constructs the full path to a specific configuration file.
 *
 * @param fileName The name of the configuration file (e.g., "config.toml").
 * @return A string representing the full path to the configuration file.
 */
FORCE_INLINE auto getConfigPath(std::string fileName) -> std::string
{
  return getBaseConfigPath() + fileName;
}

FORCE_INLINE auto getCachePath() -> const std::string
{
  const std::string cacheFilePath = getHomeDir() + "/.cache/inLimbo/";

  return cacheFilePath;
}

} // namespace utils
