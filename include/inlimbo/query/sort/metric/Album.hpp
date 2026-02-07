#pragma once

#include "Base.hpp"
#include "query/sort/Stats.hpp"

namespace query::sort::metric
{

enum class AlbumMetric
{
#define X(name, str, tag) name,
#include "config/defs/AlbumMetrics.def"
#undef X

  COUNT
};

struct AlbumLexicographicAsc
{
};
struct AlbumLexicographicDesc
{
};

struct AlbumYearAsc
{
};
struct AlbumYearDesc
{
};

struct AlbumTrackCountAsc
{
};
struct AlbumTrackCountDesc
{
};

template <>
struct Comparator<AlbumLexicographicAsc>
{
  static auto cmp(const Stats&, const Artist*, auto& A, auto& B) -> bool
  {
    auto a = std::string_view(A.first);
    auto b = std::string_view(B.first);

    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                        [](unsigned char ac, unsigned char bc) -> bool
                                        { return std::tolower(ac) < std::tolower(bc); });
  }
};

template <>
struct Comparator<AlbumLexicographicDesc>
{
  static auto cmp(const Stats&, const Artist*, auto& A, auto& B) -> bool
  {
    auto a = std::string_view(A.first);
    auto b = std::string_view(B.first);

    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                        [](unsigned char ac, unsigned char bc) -> bool
                                        { return std::tolower(ac) > std::tolower(bc); });
  }
};

template <>
struct Comparator<AlbumYearAsc>
{
  static auto cmp(const Stats& s, const Artist* artist, auto& A, auto& B) -> bool
  {
    const Artist& artistVal = *artist;

    auto keyA = Stats::AlbumKey{artistVal, A.first};
    auto keyB = Stats::AlbumKey{artistVal, B.first};

    auto itA = s.albumYear.find(keyA);
    auto itB = s.albumYear.find(keyB);

    const int yearA = (itA != s.albumYear.end()) ? itA->second : 0;
    const int yearB = (itB != s.albumYear.end()) ? itB->second : 0;

    return yearA < yearB;
  }
};

template <>
struct Comparator<AlbumTrackCountDesc>
{
  static auto cmp(const Stats& s, const Artist* artist, auto& A, auto& B) -> bool
  {
    const Artist& artistVal = *artist;

    auto keyA = Stats::AlbumKey{artistVal, A.first};
    auto keyB = Stats::AlbumKey{artistVal, B.first};

    auto itA = s.albumTrackCount.find(keyA);
    auto itB = s.albumTrackCount.find(keyB);

    const size_t countA = (itA != s.albumTrackCount.end()) ? itA->second : 0;
    const size_t countB = (itB != s.albumTrackCount.end()) ? itB->second : 0;

    return countA > countB;
  }
};

template <>
struct Comparator<AlbumYearDesc>
{
  static auto cmp(const Stats& s, const Artist* artist, auto& A, auto& B) -> bool
  {
    const Artist& artistVal = *artist;

    auto keyA = Stats::AlbumKey{artistVal, A.first};
    auto keyB = Stats::AlbumKey{artistVal, B.first};

    auto itA = s.albumYear.find(keyA);
    auto itB = s.albumYear.find(keyB);

    const int yearA = (itA != s.albumYear.end()) ? itA->second : 0;
    const int yearB = (itB != s.albumYear.end()) ? itB->second : 0;

    return yearA > yearB;
  }
};

template <>
struct Comparator<AlbumTrackCountAsc>
{
  static auto cmp(const Stats& s, const Artist* artist, auto& A, auto& B) -> bool
  {
    const Artist& artistVal = *artist;

    auto keyA = Stats::AlbumKey{artistVal, A.first};
    auto keyB = Stats::AlbumKey{artistVal, B.first};

    auto itA = s.albumTrackCount.find(keyA);
    auto itB = s.albumTrackCount.find(keyB);

    const size_t countA = (itA != s.albumTrackCount.end()) ? itA->second : 0;
    const size_t countB = (itB != s.albumTrackCount.end()) ? itB->second : 0;

    return countA < countB;
  }
};

} // namespace query::sort::metric
