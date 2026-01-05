#pragma once

#include "Config.hpp"
#include "Env-Vars.hpp"
#include "InLimbo-Types.hpp"
#include "utils/string/SmallString.hpp"
#include <stdexcept>

namespace utils
{

FORCE_INLINE auto getHomeDir() -> const utils::string::SmallString
{
  cstr homeDir = getenv("HOME");
  if (!homeDir)
  {
    throw std::runtime_error("HOME environment variable not found! Exiting program...");
  }

  return utils::string::SmallString(homeDir);
}

FORCE_INLINE auto getBaseConfigPath() -> utils::string::SmallString
{
  cstr customConfigHome = getenv(INLIMBO_CUSTOM_CONFIG_ENV);
  if (customConfigHome)
  {
    return {customConfigHome};
  }

  utils::string::SmallString fin = getHomeDir();
  fin += "/.config/inLimbo/";

  return fin;
}

FORCE_INLINE auto getConfigPathWithFile(utils::string::SmallString fileName)
  -> utils::string::SmallString
{
  utils::string::SmallString baseConfigPath = getBaseConfigPath();
  baseConfigPath += fileName;

  return baseConfigPath;
}

FORCE_INLINE auto getCachePath() -> const utils::string::SmallString
{
  utils::string::SmallString cacheFilePath = getHomeDir();
  cacheFilePath += "/.cache/inLimbo/";

  return cacheFilePath;
}

FORCE_INLINE auto getCachePathWithFile(utils::string::SmallString fileName)
  -> utils::string::SmallString
{
  utils::string::SmallString baseCachePath = getCachePath();
  baseCachePath += fileName;

  return baseCachePath;
}

} // namespace utils
