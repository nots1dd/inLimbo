#pragma once

#include <string>
#include <string_view>

namespace utils::map
{

// Hash generation possible for string_view, cstr -> without allocating std::string
struct TransparentHash
{
  using is_transparent = void;

  auto operator()(std::string_view s) const noexcept -> size_t
  {
    return std::hash<std::string_view>{}(s);
  }

  auto operator()(const std::string& s) const noexcept -> size_t
  {
    return std::hash<std::string_view>{}(std::string_view{s});
  }
};

// same logic here, comparisons with string_view, string and cstr without allocating std::string
// explicitly.
struct TransparentEq
{
  using is_transparent = void;

  auto operator()(const std::string& a, const std::string& b) const noexcept -> bool
  {
    return a == b;
  }

  // heterogeneous comparisons
  auto operator()(std::string_view a, std::string_view b) const noexcept -> bool { return a == b; }
  auto operator()(const std::string& a, std::string_view b) const noexcept -> bool
  {
    return a == b;
  }
  auto operator()(std::string_view a, const std::string& b) const noexcept -> bool
  {
    return a == b;
  }
};

} // namespace utils::map
