#pragma once

#include "InLimbo-Types.hpp"

namespace query::sort
{

struct Stats
{
  using AlbumKey = std::pair<Artist, Album>;

  ankerl::unordered_dense::map<Artist, size_t> artistAlbumCount;
  ankerl::unordered_dense::map<Artist, size_t> artistTrackCount;

  ankerl::unordered_dense::map<AlbumKey, int>    albumYear;
  ankerl::unordered_dense::map<AlbumKey, size_t> albumTrackCount;
};

auto buildStats(const SongMap& map) -> Stats;

} // namespace query::sort
