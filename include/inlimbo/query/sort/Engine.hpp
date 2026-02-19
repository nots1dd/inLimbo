#pragma once

#include "InLimbo-Types.hpp"
#include "query/sort/Stats.hpp"
#include "query/sort/metric/Album.hpp"
#include "query/sort/metric/Artist.hpp"
#include "query/sort/metric/Base.hpp"
#include "query/sort/metric/Disc.hpp"
#include "query/sort/metric/Track.hpp"

#include <algorithm>
#include <array>
#include <vector>

namespace query::sort
{

using SortApplyFn = void (*)(SongMap&);

struct RuntimeSortPlan
{
  metric::ArtistMetric artist = metric::ArtistMetric::LexAsc;
  metric::AlbumMetric  album  = metric::AlbumMetric::LexAsc;
  metric::DiscMetric   disc   = metric::DiscMetric::DiscAsc;
  metric::TrackMetric  track  = metric::TrackMetric::TrackAsc;
};

template <typename ArtistTag, typename AlbumTag, typename DiscTag, typename TrackTag>
inline void apply(SongMap& map)
{
  auto stats = sort::buildStats(map);

  std::vector<const SongMap::value_type*>  artistBuf;
  std::vector<const AlbumMap::value_type*> albumBuf;
  std::vector<const DiscMap::value_type*>  discBuf;
  std::vector<const TrackMap::value_type*> trackBuf;

  artistBuf.reserve(map.size());
  for (auto& e : map)
    artistBuf.push_back(&e);

  std::ranges::sort(artistBuf, [&](auto* A, auto* B) -> bool
                    { return metric::Comparator<ArtistTag>::cmp(stats, *A, *B); });

  SongMap newMap;
  newMap.reserve(map.size());

  for (auto* artistPair : artistBuf)
  {
    const auto& [artist, albums] = *artistPair;

    albumBuf.clear();
    albumBuf.reserve(albums.size());

    for (auto& e : albums)
      albumBuf.push_back(&e);

    std::ranges::sort(albumBuf, [&](auto* A, auto* B) -> bool
                      { return metric::Comparator<AlbumTag>::cmp(stats, &artist, *A, *B); });

    for (auto* albumPair : albumBuf)
    {
      const auto& [album, discs] = *albumPair;
      auto& newDiscs             = newMap[artist][album];

      discBuf.clear();
      discBuf.reserve(discs.size());

      for (auto& e : discs)
        discBuf.push_back(&e);

      std::ranges::sort(discBuf, [&](auto* A, auto* B) -> bool
                        { return metric::Comparator<DiscTag>::cmp(*A, *B); });

      for (auto* discPair : discBuf)
      {
        const auto& [disc, tracks] = *discPair;
        auto& newTracks            = newDiscs[disc];

        trackBuf.clear();
        trackBuf.reserve(tracks.size());

        for (auto& e : tracks)
          trackBuf.push_back(&e);

        std::ranges::sort(trackBuf, [&](auto* A, auto* B) -> bool
                          { return metric::Comparator<TrackTag>::cmp(*A, *B); });

        for (auto* trackPair : trackBuf)
        {
          const auto& [track, inodeMap] = *trackPair;
          newTracks[track]              = inodeMap;
        }
      }
    }
  }

  map = std::move(newMap);
}

template <metric::ArtistMetric M>
struct ArtistTagFromEnum;
template <metric::AlbumMetric M>
struct AlbumTagFromEnum;
template <metric::DiscMetric M>
struct DiscTagFromEnum;
template <metric::TrackMetric M>
struct TrackTagFromEnum;

#define X(name, str, tag)                              \
  template <>                                          \
  struct ArtistTagFromEnum<metric::ArtistMetric::name> \
  {                                                    \
    using type = metric::tag;                          \
  };
#include "defs/config/ArtistMetrics.def"
#undef X

#define X(name, str, tag)                            \
  template <>                                        \
  struct AlbumTagFromEnum<metric::AlbumMetric::name> \
  {                                                  \
    using type = metric::tag;                        \
  };
#include "defs/config/AlbumMetrics.def"
#undef X

#define X(name, str, tag)                          \
  template <>                                      \
  struct DiscTagFromEnum<metric::DiscMetric::name> \
  {                                                \
    using type = metric::tag;                      \
  };
#include "defs/config/DiscMetrics.def"
#undef X

#define X(name, str, tag)                            \
  template <>                                        \
  struct TrackTagFromEnum<metric::TrackMetric::name> \
  {                                                  \
    using type = metric::tag;                        \
  };
#include "defs/config/TrackMetrics.def"
#undef X

constexpr size_t ArtistCount = (size_t)metric::ArtistMetric::COUNT;
constexpr size_t AlbumCount  = (size_t)metric::AlbumMetric::COUNT;
constexpr size_t DiscCount   = (size_t)metric::DiscMetric::COUNT;
constexpr size_t TrackCount  = (size_t)metric::TrackMetric::COUNT;

constexpr size_t TOTAL = ArtistCount * AlbumCount * DiscCount * TrackCount;

template <size_t Index>
constexpr auto buildOne() -> SortApplyFn
{
  constexpr size_t A = Index / (AlbumCount * DiscCount * TrackCount);
  constexpr size_t B = (Index / (DiscCount * TrackCount)) % AlbumCount;
  constexpr size_t C = (Index / TrackCount) % DiscCount;
  constexpr size_t D = Index % TrackCount;

  using AT = typename ArtistTagFromEnum<static_cast<metric::ArtistMetric>(A)>::type;
  using BT = typename AlbumTagFromEnum<static_cast<metric::AlbumMetric>(B)>::type;
  using CT = typename DiscTagFromEnum<static_cast<metric::DiscMetric>(C)>::type;
  using DT = typename TrackTagFromEnum<static_cast<metric::TrackMetric>(D)>::type;

  return +[](SongMap& map) { apply<AT, BT, CT, DT>(map); };
}

template <size_t... I>
constexpr auto buildDispatch(std::index_sequence<I...>)
{
  return std::array<SortApplyFn, TOTAL>{buildOne<I>()...};
}

void applyRuntimeSortPlan(SongMap& map, const RuntimeSortPlan& plan);

} // namespace query::sort
