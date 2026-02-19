#include "config/sort/Model.hpp"
#include "StackTrace.hpp"
#include "config/sort/ParseMetrics.hpp"

namespace config::sort
{

auto loadRuntimeSortPlan() -> query::sort::RuntimeSortPlan
{
  RECORD_FUNC_TO_BACKTRACE("config::sort::loadRuntimeSortPlan");
  using namespace tomlparser;

  query::sort::RuntimeSortPlan plan;

  plan.artist = loadMetricOrFallback("sort", "artist", "lex_asc", DefaultArtistMetric,
                                     config::sort::parseArtistPlan, "artist");

  plan.album = loadMetricOrFallback("sort", "album", "lex_asc", DefaultAlbumMetric,
                                    config::sort::parseAlbumPlan, "album");

  plan.disc = loadMetricOrFallback("sort", "disc", "disc_asc", DefaultDiscMetric,
                                   config::sort::parseDiscPlan, "disc");

  plan.track = loadMetricOrFallback("sort", "track", "track_asc", DefaultTrackMetric,
                                    config::sort::parseTrackPlan, "track");

  return plan;
}

} // namespace config::sort
