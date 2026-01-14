#include "utils/string/Transforms.hpp"

namespace utils::string::transform
{

auto tolower_ascii(const char* s) noexcept -> SmallString
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

auto toupper_ascii(const char* s) noexcept -> SmallString
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
