#pragma once

#include <cctype>
#include <string>

namespace frontend::tui::detail
{

inline auto lrcIsSectionHeader(const std::string& token) -> bool
{
  if (token.size() < 3)
    return false;

  if (token.front() != '[' || token.back() != ']')
    return false;

  // Avoid timestamps like [01:23.45] being mistaken for section headers
  if (token.size() > 2 && std::isdigit(token[1]))
    return false;

  return true;
}

inline auto lrcIsTimestamp(const std::string& line, size_t pos) -> bool
{
  if (pos + 9 >= line.size())
    return false;

  return line[pos] == '[' && std::isdigit(line[pos + 1]) && std::isdigit(line[pos + 2]) &&
         line[pos + 3] == ':' && std::isdigit(line[pos + 4]) && std::isdigit(line[pos + 5]);
}

inline auto lrcStripTags(const std::string& line) -> std::string
{
  std::string result;
  result.reserve(line.size());

  size_t i = 0;

  while (i < line.size())
  {
    if (line[i] == '[')
    {
      size_t close = line.find(']', i);
      if (close != std::string::npos)
      {
        if (lrcIsTimestamp(line, i))
        {
          i = close + 1;
          continue;
        }

        if (line.compare(i + 1, 3, "ar:") == 0 || line.compare(i + 1, 3, "ti:") == 0 ||
            line.compare(i + 1, 3, "al:") == 0 || line.compare(i + 1, 3, "by:") == 0)
        {
          i = close + 1;
          continue;
        }
      }
    }

    result.push_back(line[i]);
    ++i;
  }

  return result;
}

} // namespace frontend::tui::detail
