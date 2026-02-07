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

// ============================================================
// STRING → ENUM PARSERS
// Uses .def as single source of truth.
// ============================================================

inline auto parseArtistPlan(std::string_view s) -> std::optional<query::sort::metric::ArtistMetric>
{
  using namespace query::sort::metric;

  static_assert(ArtistMetric_DefCount == static_cast<size_t>(ArtistMetric::COUNT),
                "ArtistMetric enum and ArtistMetrics.def mismatch");

  static const ankerl::unordered_dense::map<std::string_view, ArtistMetric> map = {

#define X(name, str, tag) {str, ArtistMetric::name},
#include "config/defs/ArtistMetrics.def"
#undef X

  };

  if (auto it = map.find(s); it != map.end())
    return it->second;

  return std::nullopt;
}

inline auto parseAlbumPlan(std::string_view s) -> std::optional<query::sort::metric::AlbumMetric>
{
  using namespace query::sort::metric;

  static_assert(AlbumMetric_DefCount == static_cast<size_t>(AlbumMetric::COUNT),
                "AlbumMetric enum and AlbumMetrics.def mismatch");

  static const ankerl::unordered_dense::map<std::string_view, AlbumMetric> map = {

#define X(name, str, tag) {str, AlbumMetric::name},
#include "config/defs/AlbumMetrics.def"
#undef X

  };

  if (auto it = map.find(s); it != map.end())
    return it->second;

  return std::nullopt;
}

inline auto parseTrackPlan(std::string_view s) -> std::optional<query::sort::metric::TrackMetric>
{
  using namespace query::sort::metric;

  static_assert(TrackMetric_DefCount == static_cast<size_t>(TrackMetric::COUNT),
                "TrackMetric enum and TrackMetrics.def mismatch");

  static const ankerl::unordered_dense::map<std::string_view, TrackMetric> map = {

#define X(name, str, tag) {str, TrackMetric::name},
#include "config/defs/TrackMetrics.def"
#undef X

  };

  if (auto it = map.find(s); it != map.end())
    return it->second;

  return std::nullopt;
}

} // namespace config::sort
