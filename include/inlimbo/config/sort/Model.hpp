#pragma once

#include "Logger.hpp"
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
#include "defs/config/ArtistMetrics.def"
#undef X
}();

constexpr auto DefaultAlbumMetric =
[]() -> query::sort::metric::AlbumMetric
{
#define X(name, str, tag) return query::sort::metric::AlbumMetric::name;
#include "defs/config/AlbumMetrics.def"
#undef X
}();

constexpr auto DefaultDiscMetric =
[]() -> query::sort::metric::DiscMetric
{
#define X(name, str, tag) return query::sort::metric::DiscMetric::name;
#include "defs/config/DiscMetrics.def"
#undef X
}();

constexpr auto DefaultTrackMetric =
[]() -> query::sort::metric::TrackMetric
{
#define X(name, str, tag) return query::sort::metric::TrackMetric::name;
#include "defs/config/TrackMetrics.def"
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

  LOG_WARN("config::sort::RuntimeSortPlan: Invalid {} sort '{}' — falling back to '{}'", logLabel,
           str, fallbackName);

  return fallbackValue;
}

auto loadRuntimeSortPlan() -> query::sort::RuntimeSortPlan;

} // namespace config::sort
