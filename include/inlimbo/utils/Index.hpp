#pragma once

#include <cstddef>
#include <optional>

// Basic index utilities (especially for circular lists like playlists)
//
// By default they return optionals

namespace utils::index
{

[[nodiscard]] inline constexpr auto prevWrap(size_t i, size_t n) noexcept -> std::optional<size_t>
{
  if (n == 0)
    return std::nullopt;
  return (i == 0) ? (n - 1) : (i - 1);
}

[[nodiscard]] inline constexpr auto nextWrap(size_t i, size_t n) noexcept -> std::optional<size_t>
{
  if (n == 0)
    return std::nullopt;
  return (i + 1 == n) ? 0 : (i + 1);
}

} // namespace utils::index
