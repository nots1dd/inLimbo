#pragma once

#include "Config.hpp"
#include "Env-Vars.hpp"
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

// NOTE:
//
// getApp<> -> Path will resolve to PATH/inLimbo
// get<>    -> Path will resolve to PATH only.
//
// ALL RETURN TYPES ARE std::filesystem::path for convenience

namespace utils
{

namespace fs = std::filesystem;

FORCE_INLINE auto getHomePath() -> fs::path
{
  const char* homeDir = std::getenv("HOME");
  if (!homeDir)
    throw std::runtime_error("HOME environment variable not found!");

  return fs::path{homeDir};
}

FORCE_INLINE auto getAppConfigPath() -> fs::path
{
  const char* customConfigHome = std::getenv(INLIMBO_CUSTOM_CONFIG_ENV);
  if (customConfigHome)
    return fs::path{customConfigHome};

  return getHomePath() / ".config" / "inLimbo";
}

FORCE_INLINE auto getAppConfigPathWithFile(const fs::path& fileName) -> fs::path
{
  return getAppConfigPath() / fileName;
}

FORCE_INLINE auto getAppDataPath() -> fs::path
{
  if (const char* dataDir = std::getenv("XDG_DATA_HOME"))
    return fs::path{dataDir} / "inLimbo";

  return getHomePath() / ".local" / "share" / "inLimbo";
}

FORCE_INLINE auto getAppDataPathWithFile(const fs::path& fileName) -> fs::path
{
  return getAppDataPath() / fileName;
}

FORCE_INLINE auto getAppCachePath() -> fs::path { return getHomePath() / ".cache" / "inLimbo"; }

FORCE_INLINE auto getAppCachePathWithFile(const fs::path& fileName) -> fs::path
{
  return getAppCachePath() / fileName;
}

FORCE_INLINE auto getAppCacheArtPath() -> fs::path { return getAppCachePath() / "art"; }

FORCE_INLINE auto getAppCacheArtPathWithFile(const fs::path& fileName) -> fs::path
{
  return getAppCacheArtPath() / fileName;
}

} // namespace utils
