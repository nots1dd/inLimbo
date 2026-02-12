#pragma once

#include "InLimbo-Types.hpp"
#include "query/sort/Stats.hpp"
#include "query/sort/metric/Album.hpp"
#include "query/sort/metric/Artist.hpp"
#include "query/sort/metric/Base.hpp"
#include "query/sort/metric/Track.hpp"

#include <algorithm>
#include <array>
#include <tuple>
#include <vector>

namespace query::sort
{

using SortApplyFn = void (*)(SongMap&);

/*
 Runtime configuration object.

 This is filled from config (TOML or elsewhere) and represents
 which metric should be used at runtime for:

   - Artist sorting
   - Album sorting
   - Track sorting

 It does NOT perform sorting itself.
 It is only used to index into the compile-time dispatch table.
*/
struct RuntimeSortPlan
{
  metric::ArtistMetric artist = metric::ArtistMetric::LexAsc;
  metric::AlbumMetric  album  = metric::AlbumMetric::LexAsc;
  metric::TrackMetric  track  = metric::TrackMetric::TrackAsc;
};

/*
 apply<ArtistTag, AlbumTag, TrackTag>()

 This is the core sorting implementation.

 It is fully compile-time configured via template parameters.
 Each Tag type selects a Comparator specialization.

 The actual sorting work happens here:
   1. Build statistics (album counts, track counts, years, etc)
   2. Copy map layers into vectors
   3. Sort vectors using comparator logic selected by Tag types
   4. Rebuild SongMap in sorted order

 Only instantiated at compile time.
*/
template <typename ArtistTag, typename AlbumTag, typename TrackTag>
inline void apply(SongMap& map)
{
  auto stats = sort::buildStats(map);

  std::vector<const SongMap::value_type*>  artistBuf;
  std::vector<const AlbumMap::value_type*> albumBuf;
  std::vector<const TrackMap::value_type*> trackBuf;

  artistBuf.reserve(map.size());

  for (auto& e : map)
    artistBuf.push_back(&e);

  std::ranges::sort(artistBuf, [&](auto* A, auto* B)
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

    std::ranges::sort(albumBuf, [&](auto* A, auto* B)
                      { return metric::Comparator<AlbumTag>::cmp(stats, &artist, *A, *B); });

    for (auto* albumPair : albumBuf)
    {
      const auto& [album, discs] = *albumPair;
      auto& newDiscs             = newMap[artist][album];

      for (auto& [disc, tracks] : discs)
      {
        trackBuf.clear();
        trackBuf.reserve(tracks.size());

        for (auto& e : tracks)
          trackBuf.push_back(&e);

        std::ranges::sort(trackBuf, [&](auto* A, auto* B)
                          { return metric::Comparator<TrackTag>::cmp(*A, *B); });

        for (auto* trackPair : trackBuf)
        {
          const auto& [track, inodeMap] = *trackPair;
          newDiscs[disc][track]         = inodeMap;
        }
      }
    }
  }

  map = std::move(newMap);
}

/*
 makeApplyFn<AT, BT, CT>()

 Converts a template instantiation into a plain function pointer.

 The unary + forces lambda -> function pointer conversion.

 Result:
   apply<AT,BT,CT> becomes callable through SortApplyFn.
*/
template <typename AT, typename BT, typename CT>
constexpr auto makeApplyFn() -> SortApplyFn
{
  return +[](SongMap& map) -> void { apply<AT, BT, CT>(map); };
}

/*
 DEF-driven mapping from enum -> Tag type.

 Each .def file entry:
   X(EnumValue, "toml_string", ComparatorTagType)

 Generates specializations:
   ArtistMetric::LexAsc -> metric::ArtistLexicographicAsc
   etc.

 This allows compile-time selection of comparator tag types
 based on enum values.
*/

template <metric::ArtistMetric M>
struct ArtistTagFromEnum;

#define X(name, str, tag)                              \
  template <>                                          \
  struct ArtistTagFromEnum<metric::ArtistMetric::name> \
  {                                                    \
    using type = metric::tag;                          \
  };

#include "defs/config/ArtistMetrics.def"
#undef X

template <metric::AlbumMetric M>
struct AlbumTagFromEnum;

#define X(name, str, tag)                            \
  template <>                                        \
  struct AlbumTagFromEnum<metric::AlbumMetric::name> \
  {                                                  \
    using type = metric::tag;                        \
  };

#include "defs/config/AlbumMetrics.def"
#undef X

template <metric::TrackMetric M>
struct TrackTagFromEnum;

#define X(name, str, tag)                            \
  template <>                                        \
  struct TrackTagFromEnum<metric::TrackMetric::name> \
  {                                                  \
    using type = metric::tag;                        \
  };

#include "defs/config/TrackMetrics.def"
#undef X

/*
 buildEntry<AI, BI, CI>()

 Instantiates one dispatch table cell.

 Steps:
   1. Convert integer indices -> enum values
   2. Convert enum values -> Tag types (via TagFromEnum)
   3. Generate function pointer to apply<TagA, TagB, TagC>()

 All resolved at compile time.
*/
template <size_t AI, size_t BI, size_t CI>
constexpr auto buildEntry() -> SortApplyFn
{
  using namespace metric;

  constexpr auto A = static_cast<ArtistMetric>(AI);
  constexpr auto B = static_cast<AlbumMetric>(BI);
  constexpr auto C = static_cast<TrackMetric>(CI);

  using AT = typename ArtistTagFromEnum<A>::type;
  using BT = typename AlbumTagFromEnum<B>::type;
  using CT = typename TrackTagFromEnum<C>::type;

  return makeApplyFn<AT, BT, CT>();
}

/*
 buildTrackRow()

 Builds one row of Track metric combinations
 for a fixed Artist + Album combination.
*/
template <size_t AI, size_t BI, size_t... CI>
constexpr auto buildTrackRow(std::index_sequence<CI...>)
{
  return std::array<SortApplyFn, sizeof...(CI)>{buildEntry<AI, BI, CI>()...};
}

/*
 buildAlbumRow()

 Builds all Track rows for one Artist metric value.
*/
template <size_t AI, size_t... BI>
constexpr auto buildAlbumRow(std::index_sequence<BI...>)
{
  constexpr auto TrackCount = (size_t)metric::TrackMetric::COUNT;

  return std::array{buildTrackRow<AI, BI>(std::make_index_sequence<TrackCount>{})...};
}

/*
 buildDispatchTableImpl()

 Builds full 3D table:
   [Artist][Album][Track]
*/
template <size_t... AI>
constexpr auto buildDispatchTableImpl(std::index_sequence<AI...>)
{
  constexpr auto AlbumCount = (size_t)metric::AlbumMetric::COUNT;

  return std::array{buildAlbumRow<AI>(std::make_index_sequence<AlbumCount>{})...};
}

/*
 Entry point for dispatch table generation.
*/
constexpr auto buildDispatchTable()
{
  constexpr auto ArtistCount = (size_t)metric::ArtistMetric::COUNT;

  return buildDispatchTableImpl(std::make_index_sequence<ArtistCount>{});
}

/*
 Global compile-time dispatch table.

 Contains pre-instantiated sorting functions for
 every valid metric combination.
*/
using DispatchTableType = decltype(buildDispatchTable());

extern const DispatchTableType DISPATCH_TABLE;

/*
 Runtime execution path.

 Runtime plan values are used as array indices.
 This selects a precompiled sorting function and executes it.
*/
void applyRuntimeSortPlan(SongMap& map, const RuntimeSortPlan& plan);

} // namespace query::sort
