#pragma once

#include "InLimbo-Types.hpp"
#include <algorithm>
#include <vector>

namespace query::songmap::sort
{

enum class Mode
{
  None,

  ArtistAsc,
  ArtistDesc,

  AlbumAsc,
  AlbumDesc,

  YearAsc,
  YearDesc
};

namespace detail
{

struct SongFlat
{
  const Artist* artist;
  const Album*  album;
  Disc          disc;
  Track         track;
  ino_t         inode;
  const Song*   song;
};

inline auto flatten(const SongMap& map)
{
  std::vector<SongFlat> out;

  for (const auto& [artist, albums] : map)
    for (const auto& [album, discs] : albums)
      for (const auto& [disc, tracks] : discs)
        for (const auto& [track, inodeMap] : tracks)
          for (const auto& [inode, song] : inodeMap)
            out.push_back({&artist, &album, disc, track, inode, &song});

  return out;
}

inline auto rebuild(const std::vector<SongFlat>& flat)
{
  SongMap newMap;

  for (auto& e : flat)
    newMap[*e.artist][*e.album][e.disc][e.track][e.inode] = *e.song;

  return newMap;
}

} // namespace detail

inline void apply(SongMap& map, Mode mode)
{
  if (mode == Mode::None)
    return;

  auto flat = detail::flatten(map);

  switch (mode)
  {
    case Mode::ArtistAsc:
      std::ranges::sort(flat, [](auto& a, auto& b) -> auto { return *a.artist < *b.artist; });
      break;

    case Mode::ArtistDesc:
      std::ranges::sort(flat, [](auto& a, auto& b) -> auto { return *a.artist > *b.artist; });
      break;

    case Mode::AlbumAsc:
      std::ranges::sort(flat, [](auto& a, auto& b) -> auto { return *a.album < *b.album; });
      break;

    case Mode::AlbumDesc:
      std::ranges::sort(flat, [](auto& a, auto& b) -> auto { return *a.album > *b.album; });
      break;

    case Mode::YearAsc:
      std::ranges::sort(flat, [](auto& a, auto& b) -> auto
                        { return a.song->metadata.year < b.song->metadata.year; });
      break;

    case Mode::YearDesc:
      std::ranges::sort(flat, [](auto& a, auto& b) -> auto
                        { return a.song->metadata.year > b.song->metadata.year; });
      break;

    default:
      break;
  }

  map = detail::rebuild(flat);
}

} // namespace query::songmap::sort
