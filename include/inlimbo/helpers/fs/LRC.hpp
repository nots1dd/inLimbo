#pragma once

#include "InLimbo-Types.hpp"
#include "utils/fs/File.hpp"
#include "utils/fs/Paths.hpp"
#include <filesystem>
#include <fstream>
#include <optional>

namespace helpers::lrc
{

namespace fs = std::filesystem;

inline static auto genLRCFilePath(std::string_view artist, std::string_view title,
                                  std::string_view album) -> fs::path
{
  auto     base = utils::fs::getAppConfigPath();
  fs::path dir  = fs::path(base) / "LRC";

  auto safeTitle  = utils::fs::sanitize_filename(title);
  auto safeArtist = utils::fs::sanitize_filename(artist);
  auto safeAlbum  = utils::fs::sanitize_filename(album);

  std::string baseName = safeTitle + "-" + safeArtist + "-" + safeAlbum;

  return fs::absolute(dir / (baseName + ".lrc"));
}

inline static auto tryReadCachedLRC(const std::filesystem::path& path) -> std::optional<Lyrics>
{
  try
  {
    if (!std::filesystem::exists(path))
      return std::nullopt;

    std::ifstream in(path, std::ios::binary);
    if (!in)
      return std::nullopt;

    std::ostringstream ss;
    ss << in.rdbuf();

    return ss.str();
  }
  catch (...)
  {
    return std::nullopt;
  }
}

} // namespace helpers::lrc
