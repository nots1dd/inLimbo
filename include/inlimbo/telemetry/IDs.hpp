#pragma once

#include "InLimbo-Types.hpp"
#include <string_view>

namespace telemetry
{

using SongID    = ui64;
using ArtistID  = ui64;
using AlbumID   = ui64;
using GenreID   = ui64;
using Timestamp = ui64; // unix seconds

constexpr auto fnv1a_64(std::string_view s) noexcept -> ui64
{
  ui64 hash = 14695981039346656037ull;
  for (unsigned char c : s)
  {
    hash ^= c;
    hash *= 1099511628211ull;
  }
  return hash;
}

inline auto makeArtistID(const Metadata& m) noexcept -> ArtistID { return fnv1a_64(m.artist); }

inline auto makeGenreID(const Metadata& m) noexcept -> GenreID { return fnv1a_64(m.genre); }

inline auto makeAlbumID(const Metadata& m) noexcept -> AlbumID
{
  // artist + album defines album identity
  std::string tmp;
  tmp.reserve(m.artist.size() + m.album.size() + 1);
  tmp += m.artist;
  tmp += '\n';
  tmp += m.album;
  return fnv1a_64(tmp);
}

inline auto makeSongID(const Metadata& m) noexcept -> SongID
{
  // BEST option: filesystem path (unique + stable)
  if (!m.filePath.empty())
    return fnv1a_64(m.filePath);

  // fallback: logical identity
  std::string tmp;
  tmp.reserve(128);
  tmp += m.artist;
  tmp += '\n';
  tmp += m.album;
  tmp += '\n';
  tmp += m.title;
  tmp += '\n';
  tmp += std::to_string(m.track);
  return fnv1a_64(tmp);
}

} // namespace telemetry
