#pragma once

#include "Config.hpp"
#include "utils/string/SmallString.hpp"
#include <string>

namespace utils::string::transform
{

// ---------------- single-char helpers ----------------

INLIMBO_API_CPP constexpr auto fast_tolower_ascii(char c) noexcept -> char
{
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

INLIMBO_API_CPP constexpr auto fast_toupper_ascii(char c) noexcept -> char
{
  return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 32) : c;
}

// ---------------- string helpers ----------------

INLIMBO_API_CPP auto tolower_ascii(const char* s) noexcept -> SmallString;
INLIMBO_API_CPP auto toupper_ascii(const char* s) noexcept -> SmallString;

INLIMBO_API_CPP auto trim(const std::string& s, std::size_t max) -> std::string;

} // namespace utils::string::transform
