#include "utils/string/Equals.hpp"
#include "utils/string/Transforms.hpp"
#include <cstddef>
#include <string>

namespace utils::string
{

auto icompare(std::string_view a, std::string_view b) -> bool
{
  auto ai = a.begin();
  auto bi = b.begin();

  for (; ai != a.end() && bi != b.end(); ++ai, ++bi)
  {
    char ac = transform::fast_tolower_ascii((unsigned char)*ai);
    char bc = transform::fast_tolower_ascii((unsigned char)*bi);

    if (ac != bc)
      return ac < bc;
  }

  return a.size() < b.size();
}

auto isEquals(const std::string& a, const std::string& b) noexcept -> bool
{
  if (a.size() != b.size())
    return false;

  const char*  pa = a.data();
  const char*  pb = b.data();
  const size_t n  = a.size();

  for (size_t i = 0; i < n; ++i)
  {
    char ca = transform::fast_tolower_ascii(pa[i]);
    char cb = transform::fast_tolower_ascii(pb[i]);
    if (ca != cb)
      return false;
  }
  return true;
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
