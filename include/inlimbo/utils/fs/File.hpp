#pragma once

#include <string>

namespace utils::fs
{

inline static auto sanitize_filename(std::string_view in) -> std::string
{
  std::string out;
  out.reserve(in.size());

  for (char c : in)
  {
    if (std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.')
      out.push_back(c);
    else if (std::isspace((unsigned char)c))
      out.push_back('_');
  }

  if (out.empty())
    out = "unknown";

  return out;
}

} // namespace utils::fs
