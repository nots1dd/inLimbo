#pragma once

#include "Base.hpp"

namespace query::sort::metric
{

enum class DiscMetric
{
#define X(name, str, tag) name,
#include "defs/config/DiscMetrics.def"
#undef X

  COUNT
};

struct DiscNumberAsc
{
};
struct DiscNumberDesc
{
};

template <>
struct Comparator<DiscNumberAsc>
{
  static auto cmp(auto& A, auto& B) -> bool { return A.first < B.first; }
};

template <>
struct Comparator<DiscNumberDesc>
{
  static auto cmp(auto& A, auto& B) -> bool { return A.first > B.first; }
};

} // namespace query::sort::metric
