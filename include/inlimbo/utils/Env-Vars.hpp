#pragma once

#include <string>
#include "Logger.hpp"

#define CUSTOM_CONFIG_MACRO "INLIMBO_CONFIG_HOME" /**< Custom config.toml macro setup */

namespace utils {

auto getHomeDir() -> const std::string 
{
  const char* homeDir = getenv("HOME");
  if (!homeDir)
  {
    LOG_ERROR("HOME environment variable not found!");
    exit(EXIT_FAILURE);
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
auto getBaseConfigPath() -> std::string
{
  const char* customConfigHome = getenv(CUSTOM_CONFIG_MACRO);
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
auto getConfigPath(std::string fileName) -> std::string { return getBaseConfigPath() + fileName; }

auto getCachePath() -> const std::string
{
  const std::string cacheFilePath = getHomeDir() + "/.cache/inLimbo/";

  return cacheFilePath;
}

}
