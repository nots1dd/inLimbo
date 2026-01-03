#pragma once

#include "Config.hpp"
#include <string>

namespace utils::string
{

inline INLIMBO_API_CPP constexpr auto fast_tolower_ascii(char c) noexcept -> char
{
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + 32) : c;
}

INLIMBO_API_CPP auto isEquals(const std::string& a, const std::string& b) noexcept -> bool;

// Optimized "contains-like" equality for situations where you already know one side is lowercase
INLIMBO_API_CPP auto isEqualsPrelowered(const std::string& lowerA, const std::string& b) noexcept
  -> bool;

} // namespace utils::string
