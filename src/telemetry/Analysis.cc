#include "telemetry/Analysis.hpp"
#include <cmath>

namespace telemetry::analysis
{

auto recencyWeight(Timestamp last, Timestamp now) -> double
{
  constexpr double halfLife = 7 * 86400.0; // 7 days
  return std::exp(-(double)(now - last) / halfLife);
}

auto mostPlayedSong(const Store& s) -> SongID
{
  SongID best = 0;
  ui32   max  = 0;

  for (auto& [id, st] : s.songs())
    if (st.playCount > max)
      best = id, max = st.playCount;

  return best;
}

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

} // namespace telemetry::analysis
