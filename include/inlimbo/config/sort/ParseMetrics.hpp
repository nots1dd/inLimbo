#pragma once

#include "query/sort/metric/Album.hpp"
#include "query/sort/metric/Artist.hpp"
#include "query/sort/metric/Track.hpp"

#include <ankerl/unordered_dense.h>
#include <optional>
#include <string_view>

namespace config::sort
{

// DEF COUNT VALIDATION:
// Ensures enum and .def stay in sync at compile time.

constexpr auto ArtistMetric_DefCount = []() -> size_t
{
  size_t n = 0;

#define X(name, str, tag) ++n;
#include "config/defs/ArtistMetrics.def"
#undef X

  return n;
}();

constexpr auto AlbumMetric_DefCount = []() -> size_t
{
  size_t n = 0;

#define X(name, str, tag) ++n;
#include "config/defs/AlbumMetrics.def"
#undef X

  return n;
}();

constexpr auto TrackMetric_DefCount = []() -> size_t
{
  size_t n = 0;

#define X(name, str, tag) ++n;
#include "config/defs/TrackMetrics.def"
#undef X

  return n;
}();

// parse the artist sorting plan found in config.toml (read the ArtistMetrics.def for available
// metrics)
auto parseArtistPlan(std::string_view s) -> std::optional<query::sort::metric::ArtistMetric>;

// parse the album sorting plan found in config.toml (AlbumMetrics.def for available metrics)
auto parseAlbumPlan(std::string_view s) -> std::optional<query::sort::metric::AlbumMetric>;

// parse the track sorting plan found in config.toml (TrackMetrics.def for available metrics)
auto parseTrackPlan(std::string_view s) -> std::optional<query::sort::metric::TrackMetric>;

} // namespace config::sort
