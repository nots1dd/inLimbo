#pragma once

#include "Config.hpp"
#include "Env-Vars.hpp"
#include "InLimbo-Types.hpp"
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

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

FORCE_INLINE auto getBaseConfigPath() -> fs::path
{
  const char* customConfigHome = std::getenv(INLIMBO_CUSTOM_CONFIG_ENV);
  if (customConfigHome)
    return fs::path{customConfigHome};

  return getHomePath() / ".config" / "inLimbo";
}

FORCE_INLINE auto getConfigPathWithFile(const fs::path& fileName) -> fs::path
{
  return getBaseConfigPath() / fileName;
}

FORCE_INLINE auto getCachePath() -> fs::path { return getHomePath() / ".cache" / "inLimbo"; }

FORCE_INLINE auto getCachePathWithFile(const fs::path& fileName) -> fs::path
{
  return getCachePath() / fileName;
}

FORCE_INLINE auto getCacheArtPath() -> fs::path { return getCachePath() / "art"; }

FORCE_INLINE auto getCacheArtPathWithFile(const fs::path& fileName) -> fs::path
{
  return getCacheArtPath() / fileName;
}

} // namespace utils
