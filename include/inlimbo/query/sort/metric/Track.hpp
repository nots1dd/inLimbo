#pragma once

#include "Base.hpp"

namespace query::sort::metric
{

enum class TrackMetric
{
#define X(name, str, tag) name,
#include "defs/config/TrackMetrics.def"
#undef X

  COUNT
};

struct TrackNumberAsc
{
};
struct TrackNumberDesc
{
};

template <>
struct Comparator<TrackNumberAsc>
{
  static auto cmp(auto& A, auto& B) -> bool { return A.first < B.first; }
};

template <>
struct Comparator<TrackNumberDesc>
{
  static auto cmp(auto& A, auto& B) -> bool { return A.first > B.first; }
};

} // namespace query::sort::metric
