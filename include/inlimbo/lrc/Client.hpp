#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "InLimbo-Types.hpp"
#include "utils/unix/net/HTTPSClient.hpp"

namespace lrc
{

inline constexpr const char* API_HOSTNAME = "lrclib.net";

struct Lyrics
{
  int id = 0;

  std::string trackName;
  std::string artistName;
  std::string albumName;

  std::optional<std::string> plainLyrics;
  std::optional<std::string> syncedLyrics;

  int duration = 0;
};

struct Query
{
  std::string        artist;
  std::string        track;
  std::string        album;
  std::optional<int> duration;
};

class Client
{
public:
  Client() = default;

  auto fetchBestMatchAndCache(const Query& q) -> utils::unix::net::Result<std::pair<Lyrics, PathStr>>;

private:
  utils::unix::net::HTTPSClient m_http;

  static auto urlEncode(std::string_view s) -> std::string;

  static auto parseLyrics(const std::string& jsonStr) -> utils::unix::net::Result<Lyrics>;
};

} // namespace lrc
