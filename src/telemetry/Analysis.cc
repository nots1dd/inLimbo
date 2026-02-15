#include "telemetry/Analysis.hpp"
#include <cmath>

namespace telemetry::analysis
{

auto totalListenTime(const Store& s) -> double
{
  double sum = 0.0;
  for (auto& [_, st] : s.songs())
    sum += st.listenSec;
  return sum;
}

auto favoriteArtist(const Store& s) -> ArtistID
{
  ArtistID best     = INVALID_TELEMETRY_ID;
  double   bestTime = 0.0;

  for (auto& [id, st] : s.artists())
    if (st.listenSec > bestTime)
      best = id, bestTime = st.listenSec;

  return best;
}

auto favoriteAlbum(const Store& s) -> AlbumID
{
  AlbumID best    = INVALID_TELEMETRY_ID;
  ui64    maxTime = 0;

  for (auto& [id, st] : s.albums())
    if (st.listenSec > maxTime)
      best = id, maxTime = st.listenSec;

  return best;
}

auto favoriteGenre(const Store& s) -> GenreID
{
  GenreID best     = INVALID_TELEMETRY_ID;
  double  bestTime = 0.0;

  for (auto& [id, st] : s.genres())
    if (st.listenSec > bestTime)
      best = id, bestTime = st.listenSec;

  return best;
}

auto mostReplayedSong(const Store& s) -> SongID
{
  SongID best     = INVALID_TELEMETRY_ID;
  ui32   maxPlays = 0;

  for (auto& [id, st] : s.songs())
    if (st.playCount > maxPlays)
      best = id, maxPlays = st.playCount;

  return best;
}

auto recencyWeight(Timestamp last, Timestamp now) -> double
{
  constexpr double halfLife = 7 * 86400.0; // 7 days
  return std::exp(-(double)(now - last) / halfLife);
}

auto hottestArtist(const Store& s, Timestamp now) -> ArtistID
{
  ArtistID best      = INVALID_TELEMETRY_ID;
  double   bestScore = 0.0;

  for (auto& [id, st] : s.artists())
  {
    double score = st.listenSec * recencyWeight(st.last, now);
    if (score > bestScore)
      best = id, bestScore = score;
  }

  return best;
}

auto hottestGenre(const Store& s, Timestamp now) -> GenreID
{
  GenreID best      = INVALID_TELEMETRY_ID;
  double  bestScore = 0.0;

  for (auto& [id, st] : s.genres())
  {
    double score = st.listenSec * recencyWeight(st.last, now);
    if (score > bestScore)
      best = id, bestScore = score;
  }

  return best;
}

auto averageSongListenTime(const Store& s) -> double
{
  double sum   = 0.0;
  ui32   count = 0;

  for (auto& [_, st] : s.songs())
  {
    sum += st.listenSec;
    count += st.playCount;
  }

  return count ? sum / count : 0.0;
}

// A song is “hot” if it is played often and was played recently
auto hottestSong(const Store& s, Timestamp now) -> SongID
{
  SongID best      = 0;
  double bestScore = 0.0;

  for (auto& [id, st] : s.songs())
  {
    double score = st.playCount * recencyWeight(st.last, now);
    if (score > bestScore)
      best = id, bestScore = score;
  }

  return best;
}

auto firstListenedSong(const Store& s) -> SongID
{
  SongID    best  = INVALID_TELEMETRY_ID;
  Timestamp first = std::numeric_limits<Timestamp>::max();

  for (auto& [id, st] : s.songs())
  {
    if (st.playCount == 0)
      continue;

    if (st.last < first)
    {
      first = st.last;
      best  = id;
    }
  }

  return best;
}

auto lastListenedSong(const Store& s) -> SongID
{
  SongID    best = INVALID_TELEMETRY_ID;
  Timestamp last = 0;

  for (auto& [id, st] : s.songs())
  {
    if (st.playCount == 0)
      continue;

    if (st.last > last)
    {
      last = st.last;
      best = id;
    }
  }

  return best;
}

} // namespace telemetry::analysis
