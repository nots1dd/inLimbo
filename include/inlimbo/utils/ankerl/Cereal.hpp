#pragma once

#include <ankerl/unordered_dense.h>
#include <cereal/cereal.hpp>

namespace cereal
{

template <class Archive, class K, class V, class Hash, class Eq, class Alloc>
void save(Archive& ar, const ankerl::unordered_dense::map<K, V, Hash, Eq, Alloc>& map)
{
  const size_t size = map.size();
  ar(size);
  for (const auto& [k, v] : map)
  {
    ar(k, v);
  }
}

template <class Archive, class K, class V, class Hash, class Eq, class Alloc>
void load(Archive& ar, ankerl::unordered_dense::map<K, V, Hash, Eq, Alloc>& map)
{
  size_t size = 0;
  ar(size);

  map.clear();
  map.reserve(size);

  for (size_t i = 0; i < size; ++i)
  {
    K k;
    V v;
    ar(k, v);
    map.emplace(std::move(k), std::move(v));
  }
}

} // namespace cereal
