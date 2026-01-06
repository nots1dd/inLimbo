#pragma once

#include "InLimbo-Types.hpp"
#include "utils/string/Equals.hpp"
#include <functional>

namespace query::song
{

using SongPredicate = std::function<bool(const Artist&, const Album&, Disc, Track, const Song&)>;

namespace sort
{

inline auto byTitle(const Title& t) -> SongPredicate
{
  return [t](auto&, auto&, auto, auto, const Song& s) -> bool
  { return utils::string::isEquals(s.metadata.title, t); };
}

inline auto byTrack(int track) -> SongPredicate
{
  return [track](auto&, auto&, auto, int trk, auto&) -> bool { return trk == track; };
}

inline auto byArtist(const Artist& a) -> SongPredicate
{
  return [a](const Artist& artist, auto&, auto, auto, auto&) -> bool
  { return utils::string::isEquals(artist, a); };
}

inline auto byAlbum(const Album& a) -> SongPredicate
{
  return [a](auto, const Album& album, auto, auto, auto&) -> bool
  { return utils::string::isEquals(album, a); };
}

inline auto byArtistDisc(const Artist& a, Disc d) -> SongPredicate
{
  return [a, d](const Artist& artist, auto&, Disc disc, auto, auto&) -> bool
  { return utils::string::isEquals(artist, a) && disc == d; };
}

inline auto byAlbumDisc(const Album& al, Disc d) -> SongPredicate
{
  return [al, d](auto, const Album& album, auto disc, auto, auto&) -> bool
  { return utils::string::isEquals(album, al) && disc == d; };
}

inline auto byDisc(Disc d) -> SongPredicate
{
  return [d](auto&, auto&, Disc disc, auto, auto&) -> bool { return disc == d; };
}

inline auto byArtistAlbum(const Artist& a, const Album& al) -> SongPredicate
{
  return [a, al](const Artist& artist, const Album& album, auto, auto, auto&) -> bool
  { return utils::string::isEquals(artist, a) && utils::string::isEquals(album, al); };
}

inline auto byYear(uint year) -> SongPredicate
{
  return [year](auto&, auto&, auto, auto, const Song& s) -> bool
  { return s.metadata.year == year; };
}

inline auto byGenre(const Genre& g) -> SongPredicate
{
  return [g](auto&, auto&, auto, auto, const Song& s) -> bool
  { return utils::string::isEquals(s.metadata.genre, g); };
}

inline auto allOf(SongPredicate a, SongPredicate b) -> SongPredicate
{
  return [=](auto&&... args) -> auto { return a(args...) && b(args...); };
}

inline auto anyOf(SongPredicate a, SongPredicate b) -> SongPredicate
{
  return [=](auto&&... args) -> auto { return a(args...) || b(args...); };
}

inline auto notOf(SongPredicate p) -> SongPredicate
{
  return [=](auto&&... args) -> auto { return !p(args...); };
}

} // namespace sort

} // namespace query::song
