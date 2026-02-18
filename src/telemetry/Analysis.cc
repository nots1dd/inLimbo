#include "telemetry/Analysis.hpp"
#include <cmath>
#include <limits>

namespace telemetry::analysis
{

// ------------------------------------------------------------
// Aggregates
// ------------------------------------------------------------

auto totalListenTime(const Store& s) -> double
{
  double sum = 0.0;
  for (auto& [_, st] : s.songs())
  {
    sum += st.listenSec;
  }
  return sum;
}

// ------------------------------------------------------------
// Favorites (committed only)
// ------------------------------------------------------------

auto favoriteArtist(const Store& s) -> ArtistID
{
  ArtistID best     = INVALID_TELEMETRY_ID;
  double   bestTime = 0.0;

  for (auto& [id, st] : s.artists())
  {
    if (st.playCount == 0)
      continue;

    if (st.listenSec > bestTime)
      best = id, bestTime = st.listenSec;
  }

  return best;
}

auto favoriteAlbum(const Store& s) -> AlbumID
{
  AlbumID best     = INVALID_TELEMETRY_ID;
  double  bestTime = 0.0;

  for (auto& [id, st] : s.albums())
  {
    if (st.playCount == 0)
      continue;

    if (st.listenSec > bestTime)
      best = id, bestTime = st.listenSec;
  }

  return best;
}

auto favoriteGenre(const Store& s) -> GenreID
{
  GenreID best     = INVALID_TELEMETRY_ID;
  double  bestTime = 0.0;

  for (auto& [id, st] : s.genres())
  {
    if (st.playCount == 0)
      continue;

    if (st.listenSec > bestTime)
      best = id, bestTime = st.listenSec;
  }

  return best;
}

// ------------------------------------------------------------
// Replays
// ------------------------------------------------------------

auto mostReplayedSong(const Store& s) -> SongID
{
  SongID best     = INVALID_TELEMETRY_ID;
  ui32   maxPlays = 0;

  for (auto& [id, st] : s.songs())
  {
    if (st.playCount > maxPlays)
      best = id, maxPlays = st.playCount;
  }

  return best;
}

// ------------------------------------------------------------
// Recency
// ------------------------------------------------------------

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
    if (st.playCount == 0)
      continue;

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
    if (st.playCount == 0)
      continue;

    double score = st.listenSec * recencyWeight(st.last, now);
    if (score > bestScore)
      best = id, bestScore = score;
  }

  return best;
}

// ------------------------------------------------------------
// Averages
// ------------------------------------------------------------

auto averageSongListenTime(const Store& s) -> double
{
  double sum   = 0.0;
  ui32   plays = 0;

  for (auto& [_, st] : s.songs())
  {
    if (st.playCount == 0)
      continue;

    sum += st.listenSec;
    plays += st.playCount;
  }

  return plays ? sum / plays : 0.0;
}

// ------------------------------------------------------------
// Hot songs (play count + recency)
// ------------------------------------------------------------

auto hottestSong(const Store& s, Timestamp now) -> SongID
{
  SongID best      = INVALID_TELEMETRY_ID;
  double bestScore = 0.0;

  for (auto& [id, st] : s.songs())
  {
    if (st.playCount == 0)
      continue;

    double score = static_cast<double>(st.playCount) * recencyWeight(st.last, now);

    if (score > bestScore)
      best = id, bestScore = score;
  }

  return best;
}

// ------------------------------------------------------------
// First / last listened (committed only)
// ------------------------------------------------------------

auto firstListenedSong(const Store& s) -> SongID
{
  SongID    best  = INVALID_TELEMETRY_ID;
  Timestamp first = std::numeric_limits<Timestamp>::max();

  for (auto& [id, st] : s.songs())
  {
    if (st.playCount == 0)
      continue;

    if (st.first < first)
    {
      first = st.first;
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
