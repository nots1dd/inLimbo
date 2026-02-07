#pragma once

#include <optional>
#include <string_view>

namespace utils::enum_reflect
{

template <typename E>
struct Traits;

template <typename E>
constexpr auto fromString(std::string_view s) -> std::optional<E>
{
  constexpr auto& names  = Traits<E>::names;
  constexpr auto& values = Traits<E>::values;

  for (size_t i = 0; i < names.size(); ++i)
    if (names[i] == s)
      return values[i];

  return std::nullopt;
}

template <typename E>
constexpr auto toString(E v) -> std::string_view
{
  constexpr auto& names  = Traits<E>::names;
  constexpr auto& values = Traits<E>::values;

  for (size_t i = 0; i < values.size(); ++i)
    if (values[i] == v)
      return names[i];

  return {};
}

} // namespace utils::enum_reflect
