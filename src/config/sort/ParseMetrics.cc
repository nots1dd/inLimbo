#include "config/sort/ParseMetrics.hpp"
#include "query/sort/metric/Disc.hpp"

namespace config::sort
{

using namespace query::sort::metric;

// ============================================================
// Static lookup maps (exist once in program)
// ============================================================

static const ankerl::unordered_dense::map<std::string_view, ArtistMetric> ARTIST_MAP = {

#define X(name, str, tag) {str, ArtistMetric::name},
#include "defs/config/ArtistMetrics.def"
#undef X

};

static const ankerl::unordered_dense::map<std::string_view, AlbumMetric> ALBUM_MAP = {

#define X(name, str, tag) {str, AlbumMetric::name},
#include "defs/config/AlbumMetrics.def"
#undef X

};

static const ankerl::unordered_dense::map<std::string_view, TrackMetric> TRACK_MAP = {

#define X(name, str, tag) {str, TrackMetric::name},
#include "defs/config/TrackMetrics.def"
#undef X

};

static const ankerl::unordered_dense::map<std::string_view, DiscMetric> DISC_MAP = {

#define X(name, str, tag) {str, DiscMetric::name},
#include "defs/config/DiscMetrics.def"
#undef X

};

auto parseArtistPlan(std::string_view s) -> std::optional<ArtistMetric>
{
  static_assert(ArtistMetric_DefCount == static_cast<size_t>(ArtistMetric::COUNT),
                "ArtistMetric enum and ArtistMetrics.def mismatch");

  if (auto it = ARTIST_MAP.find(s); it != ARTIST_MAP.end())
    return it->second;

  return std::nullopt;
}

auto parseAlbumPlan(std::string_view s) -> std::optional<AlbumMetric>
{
  static_assert(AlbumMetric_DefCount == static_cast<size_t>(AlbumMetric::COUNT),
                "AlbumMetric enum and AlbumMetrics.def mismatch");

  if (auto it = ALBUM_MAP.find(s); it != ALBUM_MAP.end())
    return it->second;

  return std::nullopt;
}

auto parseTrackPlan(std::string_view s) -> std::optional<TrackMetric>
{
  static_assert(TrackMetric_DefCount == static_cast<size_t>(TrackMetric::COUNT),
                "TrackMetric enum and TrackMetrics.def mismatch");

  if (auto it = TRACK_MAP.find(s); it != TRACK_MAP.end())
    return it->second;

  return std::nullopt;
}

auto parseDiscPlan(std::string_view s) -> std::optional<DiscMetric>
{
  static_assert(DiscMetric_DefCount == static_cast<size_t>(DiscMetric::COUNT),
                "DiscMetric enum and DiscMetrics.def mismatch");

  if (auto it = DISC_MAP.find(s); it != DISC_MAP.end())
    return it->second;

  return std::nullopt;
}

} // namespace config::sort
