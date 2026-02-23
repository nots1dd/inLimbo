#pragma once

#include "Logger.hpp"
#include "query/SongMap.hpp"

namespace helpers::fuzzy
{

enum class FuzzyKind
{
  Title,
  Artist,
  Album,
  Genre
};

template <typename T>
void logFuzzyNoMatch(const char* kind, const T& query)
{
  LOG_ERROR("No fuzzy match found for kind '{}'. Provided query='{}'", kind, query);
}

template <typename T>
inline static auto logFuzzyFallback(std::string_view type, const T& input, const T& best) -> void
{
  if (!input.empty() && !best.empty() && input != best)
  {
    LOG_WARN("No exact {} match for '{}' -> using fuzzy match '{}'", type, input, best);
  }
}

template <FuzzyKind Kind, typename T, typename SongMap>
auto bestCandidate(SongMap& songMap, const T& query, const int fuzzyMaxDist,
                   const bool useFuzzySearch = true) -> T
{
  if (!useFuzzySearch)
    return query;

  if constexpr (Kind == FuzzyKind::Title)
  {
    auto song = query::songmap::read::findSongObjByTitleFuzzy(songMap, query, fuzzyMaxDist);

    if (!song)
    {
      logFuzzyNoMatch("song", query);
      return {};
    }

    const auto best = song->metadata.title;
    logFuzzyFallback("song", query, best);
    return best;
  }
  else if constexpr (Kind == FuzzyKind::Artist)
  {
    const auto best = query::songmap::read::findArtistFuzzy(songMap, query, fuzzyMaxDist);

    if (best.empty())
    {
      logFuzzyNoMatch("artist", query);
      return {};
    }

    logFuzzyFallback("artist", query, best);
    return best;
  }
  else if constexpr (Kind == FuzzyKind::Album)
  {
    const auto best = query::songmap::read::findAlbumFuzzy(songMap, query, fuzzyMaxDist);

    if (best.empty())
    {
      logFuzzyNoMatch("album", query);
      return {};
    }

    logFuzzyFallback("album", query, best);
    return best;
  }
  else if constexpr (Kind == FuzzyKind::Genre)
  {
    const auto best = query::songmap::read::findGenreFuzzy(songMap, query, fuzzyMaxDist);

    if (best.empty())
    {
      logFuzzyNoMatch("genre", query);
      return {};
    }

    logFuzzyFallback("genre", query, best);
    return best;
  }
}

template <FuzzyKind Kind, typename T>
inline static auto fuzzyResolve(const TS_SongMap& songMap, const T& input, size_t maxDist,
                                bool enableFuzzy) -> T
{
  if constexpr (std::is_same_v<T, std::optional<typename T::value_type>>)
  {
    if (!input.has_value() || input->empty())
      return input;

    auto best = bestCandidate<Kind>(songMap, *input, maxDist, enableFuzzy);
    return T{best};
  }
  else
  {
    if (input.empty())
      return input;

    return bestCandidate<Kind>(songMap, input, maxDist, enableFuzzy);
  }
}

template <FuzzyKind Kind, typename T, typename Fn>
inline static void fuzzyDispatch(const TS_SongMap& songMap, const T& input, size_t maxDist,
                                 bool enableFuzzy, Fn&& fn)
{
  auto best = fuzzyResolve<Kind>(songMap, input, maxDist, enableFuzzy);
  fn(best);
}

} // namespace helpers::fuzzy
