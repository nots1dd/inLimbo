#pragma once

#include "InLimbo-Types.hpp"
#include "Logger.hpp"
#include "config/sort/ParseMetrics.hpp"
#include "query/sort/Engine.hpp"
#include "toml/Parser.hpp"
#include <ankerl/unordered_dense.h>
#include <string_view>

namespace config::sort
{

constexpr auto DefaultArtistMetric =
[]() -> query::sort::metric::ArtistMetric
{
#define X(name, str, tag) return query::sort::metric::ArtistMetric::name;
#include "config/defs/ArtistMetrics.def"
#undef X
}();

constexpr auto DefaultAlbumMetric =
[]() -> query::sort::metric::AlbumMetric
{
#define X(name, str, tag) return query::sort::metric::AlbumMetric::name;
#include "config/defs/AlbumMetrics.def"
#undef X
}();

constexpr auto DefaultTrackMetric =
[]() -> query::sort::metric::TrackMetric
{
#define X(name, str, tag) return query::sort::metric::TrackMetric::name;
#include "config/defs/TrackMetrics.def"
#undef X
}();

template <typename EnumT, typename ParseFn>
auto loadMetricOrFallback(tomlparser::SectionView section, tomlparser::KeyView key,
                          std::string_view fallbackName, EnumT fallbackValue, ParseFn parseFn,
                          std::string_view logLabel) -> EnumT
{
  const auto str = tomlparser::Config::getString(section, key, fallbackName);

  if (auto v = parseFn(str))
    return *v;

  LOG_WARN("RuntimeSortPlan: Invalid {} sort '{}' — falling back to '{}'", logLabel, str,
           fallbackName);

  return fallbackValue;
}

template <typename Plan>
static void applyPlan(SongMap& map)
{
  query::sort::apply<Plan>(map);
}

inline auto loadRuntimeSortPlan() -> query::sort::RuntimeSortPlan
{
  using namespace tomlparser;

  query::sort::RuntimeSortPlan plan;

  plan.artist = loadMetricOrFallback("sort", "artist", "lex_asc", DefaultArtistMetric,
                                     config::sort::parseArtistPlan, "artist");

  plan.album = loadMetricOrFallback("sort", "album", "lex_asc", DefaultAlbumMetric,
                                    config::sort::parseAlbumPlan, "album");

  plan.track = loadMetricOrFallback("sort", "track", "track_asc", DefaultTrackMetric,
                                    config::sort::parseTrackPlan, "track");

  return plan;
}

} // namespace config::sort
