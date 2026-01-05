#pragma once

#include "Config.hpp"
#include "utils/string/SmallString.hpp"

namespace utils::string::transform
{

// ---------------- single-char helpers ----------------

inline INLIMBO_API_CPP constexpr auto fast_tolower_ascii(char c) noexcept -> char
{
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

inline INLIMBO_API_CPP constexpr auto fast_toupper_ascii(char c) noexcept -> char
{
  return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 32) : c;
}

inline INLIMBO_API_CPP auto tolower_ascii(const char* s) noexcept -> SmallString
{
  SmallString out;

  if (!s)
    return out;

  out += s;

  char* data = out.data();
  for (uint32_t i = 0; i < out.size(); ++i)
  {
    data[i] = fast_tolower_ascii(data[i]);
  }

  return out;
}

inline INLIMBO_API_CPP auto toupper_ascii(const char* s) noexcept -> SmallString
{
  SmallString out;

  if (!s)
    return out;

  out += s;

  char* data = out.data();
  for (uint32_t i = 0; i < out.size(); ++i)
  {
    data[i] = fast_toupper_ascii(data[i]);
  }

  return out;
}

} // namespace utils::string::transform
