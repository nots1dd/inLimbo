#include "lrc/Client.hpp"

#include "Logger.hpp"
#include "utils/fs/File.hpp"
#include "utils/timer/Timer.hpp"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "utils/fs/Paths.hpp"

namespace lrc
{

using utils::unix::net::ErrorCode;
using utils::unix::net::Result;

namespace fs = std::filesystem;

static auto make_lrc_header() -> std::string
{
  std::ostringstream oss;
  oss << "# fetched_at=" << utils::timer::timestamp_now() << "\n";
  oss << "# source=lrclib\n";
  return oss.str();
}

static auto format_lrc(const Lyrics& lyr) -> std::string
{
  std::ostringstream out;

  out << make_lrc_header();

  if (lyr.syncedLyrics && !lyr.syncedLyrics->empty())
  {
    // Already in LRC format from LRCLIB
    out << *lyr.syncedLyrics;
  }
  else if (lyr.plainLyrics)
  {
    // Fallback — convert to pseudo LRC with no timestamps
    std::istringstream iss(*lyr.plainLyrics);
    std::string        line;

    while (std::getline(iss, line))
    {
      out << "[00:00.00]" << line << "\n";
    }
  }

  return out.str();
}

static auto cache_lrclib_files(const Query& q, const std::string& rawJson, const Lyrics& lyr)
  -> std::string
{
  try
  {
    auto     base = utils::fs::getAppConfigPath();
    fs::path dir  = fs::path(base) / "LRC";

    fs::create_directories(dir);

    auto title  = utils::fs::sanitize_filename(q.track);
    auto artist = utils::fs::sanitize_filename(q.artist);
    auto album  = utils::fs::sanitize_filename(q.album);

    std::string baseName = title + "-" + artist + "-" + album;

    const fs::path jsonPath = dir / (baseName + ".json");
    const fs::path lrcPath  = dir / (baseName + ".lrc");

    // JSON
    {
      std::ofstream out(jsonPath, std::ios::binary);
      out << rawJson;
    }

    // LRC
    {
      std::ofstream out(lrcPath, std::ios::binary);
      out << format_lrc(lyr);
    }

    LOG_DEBUG("Cached LRCLIB JSON + LRC for {}", baseName);

    return fs::absolute(lrcPath).string();
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Failed to cache LRCLIB files: {}", e.what());
    return {};
  }
}

auto Client::urlEncode(std::string_view in) -> std::string
{
  std::string out;
  char        buf[4];

  auto isUnreserved = [](char c) -> bool
  { return std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~'; };

  for (char c : in)
  {
    if (isUnreserved(c))
      out.push_back(c);
    else
    {
      snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
      out += buf;
    }
  }

  return out;
}

auto Client::parseLyrics(const std::string& jsonStr) -> Result<Lyrics>
{
  try
  {
    auto j = nlohmann::json::parse(jsonStr);

    Lyrics l;

    l.id         = j.value("id", 0);
    l.trackName  = j.value("trackName", "");
    l.artistName = j.value("artistName", "");
    l.albumName  = j.value("albumName", "");

    if (j.contains("plainLyrics") && !j["plainLyrics"].is_null())
      l.plainLyrics = j["plainLyrics"].get<std::string>();

    if (j.contains("syncedLyrics") && !j["syncedLyrics"].is_null())
      l.syncedLyrics = j["syncedLyrics"].get<std::string>();

    l.duration = j.value("duration", 0);

    return {.value = std::move(l), .error = {}};
  }
  catch (const std::exception& e)
  {
    return {.value = {},
            .error = {.code    = ErrorCode::HTTPError,
                      .message = std::string("JSON parse failed: ") + e.what()}};
  }
}

auto Client::fetchBestMatchAndCache(const Query& q) -> Result<std::pair<Lyrics, PathStr>>
{
  std::ostringstream path;
  path << "/api/get?";

  path << "artist_name=" << urlEncode(q.artist);
  path << "&track_name=" << urlEncode(q.track);

  if (!q.album.empty())
    path << "&album_name=" << urlEncode(q.album);

  if (q.duration)
    path << "&duration=" << *q.duration;

  utils::Timer<> timer;
  timer.start();

  auto http = m_http.get(API_HOSTNAME, path.str());

  if (!http.ok())
  {
    LOG_ERROR("lrc::Client HTTPS error occurred in {:.3f} ms", timer.elapsed_ms());

    return {.value = {}, .error = http.error};
  }

  LOG_DEBUG("lrc::Client fetched data in {:.3f} ms", timer.elapsed_ms());

  auto parsed = parseLyrics(http.value);
  if (!parsed.ok())
    return {.value = {}, .error = parsed.error};

  auto lrcPath = cache_lrclib_files(q, http.value, parsed.value);

  return {.value = {std::move(parsed.value), std::move(lrcPath)}, .error = {}};
}

} // namespace lrc
