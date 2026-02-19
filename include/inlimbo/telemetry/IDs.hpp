#pragma once

#include "InLimbo-Types.hpp"
#include "utils/ankerl/Cereal.hpp"
#include "utils/string/Table.hpp"

namespace telemetry
{

using SongID    = ui32;
using ArtistID  = ui32;
using AlbumID   = ui32;
using GenreID   = ui32;
using Timestamp = ui64;

inline static constexpr ui32 INVALID_TELEMETRY_ID = 0;

struct Registry
{
  utils::string::StringTable<SongID>   titles;
  utils::string::StringTable<ArtistID> artists;
  utils::string::StringTable<AlbumID>  albums;
  utils::string::StringTable<GenreID>  genres;

  ankerl::unordered_dense::map<SongID, ArtistID> songToArtistMap;

  [[nodiscard]] inline auto songToArtist(SongID id) const -> ArtistID
  {
    auto it = songToArtistMap.find(id);
    if (it == songToArtistMap.end())
      return INVALID_TELEMETRY_ID;
    return it->second;
  }

  inline void bindSongToArtist(SongID song, ArtistID artist) { songToArtistMap[song] = artist; }

  [[nodiscard]] auto save(const std::string& path) const -> bool;
  auto               load(const std::string& path) -> bool;

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(titles, artists, albums, genres, songToArtistMap);
  }
};

} // namespace telemetry
