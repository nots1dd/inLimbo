#pragma once

#include "Config.hpp"
#include "utils/Env-Vars.hpp"
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

// NOTE:
//
// getApp<> -> Path will resolve to PATH/inLimbo
// get<>    -> Path will resolve to PATH only.
//
// ALL RETURN TYPES ARE std::filesystem::path for convenience

namespace utils::fs
{

FORCE_INLINE auto getHomePath() -> std::filesystem::path
{
  const char* homeDir = std::getenv("HOME");
  if (!homeDir)
    throw std::runtime_error("HOME environment variable not found!");

  return std::filesystem::path{homeDir};
}

FORCE_INLINE auto getAppConfigPath() -> std::filesystem::path
{
  const char* customConfigHome = std::getenv(INLIMBO_CUSTOM_CONFIG_ENV);
  if (customConfigHome)
    return std::filesystem::path{customConfigHome};

  return getHomePath() / ".config" / "inLimbo";
}

FORCE_INLINE auto getAppConfigPathWithFile(const std::filesystem::path& fileName)
  -> std::filesystem::path
{
  return getAppConfigPath() / fileName;
}

FORCE_INLINE auto getAppDataPath() -> std::filesystem::path
{
  if (const char* dataDir = std::getenv("XDG_DATA_HOME"))
    return std::filesystem::path{dataDir} / "inLimbo";

  return getHomePath() / ".local" / "share" / "inLimbo";
}

FORCE_INLINE auto getAppDataPathWithFile(const std::filesystem::path& fileName)
  -> std::filesystem::path
{
  return getAppDataPath() / fileName;
}

FORCE_INLINE auto getAppCachePath() -> std::filesystem::path
{
  return getHomePath() / ".cache" / "inLimbo";
}

FORCE_INLINE auto getAppCachePathWithFile(const std::filesystem::path& fileName)
  -> std::filesystem::path
{
  return getAppCachePath() / fileName;
}

FORCE_INLINE auto getAppCacheArtPath() -> std::filesystem::path
{
  return getAppCachePath() / "art";
}

FORCE_INLINE auto getAppCacheArtPathWithFile(const std::filesystem::path& fileName)
  -> std::filesystem::path
{
  return getAppCacheArtPath() / fileName;
}

} // namespace utils::fs
