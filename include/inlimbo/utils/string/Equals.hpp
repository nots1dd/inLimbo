#pragma once

#include "Config.hpp"
#include <string>

namespace utils::string
{

auto icompare(std::string_view a, std::string_view b) -> bool;

INLIMBO_API_CPP auto isEquals(const std::string& a, const std::string& b) noexcept -> bool;

// Optimized "contains-like" equality for situations where you already know one side is lowercase
INLIMBO_API_CPP auto isEqualsPrelowered(const std::string& lowerA, const std::string& b) noexcept
  -> bool;

} // namespace utils::string
