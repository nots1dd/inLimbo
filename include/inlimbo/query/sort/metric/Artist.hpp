#pragma once

#include "Base.hpp"
#include "query/sort/Stats.hpp"
#include "utils/string/Equals.hpp"

namespace query::sort::metric
{

enum class ArtistMetric
{
#define X(name, str, tag) name,
#include "defs/config/ArtistMetrics.def"
#undef X

  COUNT
};

struct ArtistLexicographicAsc
{
};
struct ArtistLexicographicDesc
{
};

struct ArtistAlbumCountAsc
{
};
struct ArtistAlbumCountDesc
{
};

struct ArtistTrackCountAsc
{
};
struct ArtistTrackCountDesc
{
};

template <>
struct Comparator<ArtistLexicographicAsc>
{
  static auto cmp(const Stats&, auto& A, auto& B) -> bool
  {
    return utils::string::icompare(A.first, B.first);
  }
};

template <>
struct Comparator<ArtistLexicographicDesc>
{
  static auto cmp(const Stats&, auto& A, auto& B) -> bool
  {
    return utils::string::icompare(B.first, A.first);
  }
};

template <>
struct Comparator<ArtistAlbumCountAsc>
{
  static auto cmp(const Stats& s, auto& A, auto& B) -> bool
  {
    auto itA = s.artistAlbumCount.find(A.first);
    auto itB = s.artistAlbumCount.find(B.first);

    const size_t a = (itA != s.artistAlbumCount.end()) ? itA->second : 0;
    const size_t b = (itB != s.artistAlbumCount.end()) ? itB->second : 0;

    return a < b;
  }
};

template <>
struct Comparator<ArtistAlbumCountDesc>
{
  static auto cmp(const Stats& s, auto& A, auto& B) -> bool
  {
    auto itA = s.artistAlbumCount.find(A.first);
    auto itB = s.artistAlbumCount.find(B.first);

    const size_t a = (itA != s.artistAlbumCount.end()) ? itA->second : 0;
    const size_t b = (itB != s.artistAlbumCount.end()) ? itB->second : 0;

    return a > b;
  }
};

template <>
struct Comparator<ArtistTrackCountAsc>
{
  static auto cmp(const Stats& s, auto& A, auto& B) -> bool
  {
    auto itA = s.artistTrackCount.find(A.first);
    auto itB = s.artistTrackCount.find(B.first);

    const size_t a = (itA != s.artistTrackCount.end()) ? itA->second : 0;
    const size_t b = (itB != s.artistTrackCount.end()) ? itB->second : 0;

    return a < b;
  }
};

template <>
struct Comparator<ArtistTrackCountDesc>
{
  static auto cmp(const Stats& s, auto& A, auto& B) -> bool
  {
    auto itA = s.artistTrackCount.find(A.first);
    auto itB = s.artistTrackCount.find(B.first);

    const size_t a = (itA != s.artistTrackCount.end()) ? itA->second : 0;
    const size_t b = (itB != s.artistTrackCount.end()) ? itB->second : 0;

    return a > b;
  }
};

} // namespace query::sort::metric
