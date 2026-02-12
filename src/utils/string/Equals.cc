#include "utils/string/Equals.hpp"
#include "utils/string/Transforms.hpp"
#include "utils/string/Unicode.hpp"
#include <cstddef>
#include <string>

namespace utils::string
{

auto icompare(std::string_view a, std::string_view b) -> bool
{
  const char* ap = a.data();
  const char* bp = b.data();

  const char* ae = ap + a.size();
  const char* be = bp + b.size();

  while (ap < ae && bp < be)
  {
    char32_t ac = unicode_tolower(utf8_decode(ap, ae));
    char32_t bc = unicode_tolower(utf8_decode(bp, be));

    if (ac != bc)
      return ac < bc;
  }

  // shorter string wins
  if (ap == ae && bp == be)
    return false;

  return ap == ae;
}

auto isEquals(const std::string& a, const std::string& b) noexcept -> bool
{
  const char* ap = a.data();
  const char* bp = b.data();

  const char* ae = ap + a.size();
  const char* be = bp + b.size();

  while (ap < ae && bp < be)
  {
    char32_t ac = unicode_tolower(utf8_decode(ap, ae));
    char32_t bc = unicode_tolower(utf8_decode(bp, be));

    if (ac != bc)
      return false;
  }

  // Both must end at same time
  return (ap == ae) && (bp == be);
}

// Optimized "contains-like" equality for situations where you already know one side is lowercase
auto isEqualsPrelowered(const std::string& lowerA, const std::string& b) noexcept -> bool
{
  if (lowerA.size() != b.size())
    return false;

  const char*  pa = lowerA.data();
  const char*  pb = b.data();
  const size_t n  = lowerA.size();

  for (size_t i = 0; i < n; ++i)
  {
    char cb = transform::fast_tolower_ascii(pb[i]);
    if (pa[i] != cb)
      return false;
  }
  return true;
}

} // namespace utils::string
