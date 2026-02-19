#pragma once

namespace query::sort::metric
{

template <typename T>
struct Comparator
{
  static_assert(sizeof(T) == 0, "Comparator specialization missing for metric");
};

} // namespace query::sort::metric
