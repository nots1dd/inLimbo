#pragma once

#include "InLimbo-Types.hpp"
#include "utils/string/Table.hpp"

namespace telemetry
{

using SongID    = ui32;
using ArtistID  = ui32;
using AlbumID   = ui32;
using GenreID   = ui32;
using Timestamp = ui64; // unix seconds

inline static constexpr ui32 INVALID_TELEMETRY_ID = 0;

struct Registry
{
  utils::string::StringTable<SongID>   titles;
  utils::string::StringTable<ArtistID> artists;
  utils::string::StringTable<AlbumID>  albums;
  utils::string::StringTable<GenreID>  genres;

  [[nodiscard]] auto save(const std::string& path) const -> bool;
  auto               load(const std::string& path) -> bool;

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(titles, artists, albums, genres);
  }
};

} // namespace telemetry
