#ifndef MISC_HEADER
#define MISC_HEADER

#include <cctype>
#include <string>
#include <vector>

std::vector<std::string> formatLyrics(const std::string& lyrics)
{
  std::vector<std::string> lines;
  std::string              currentLine;
  bool                     lastWasUppercase = false;
  bool                     insideBrackets   = false;
  bool                     wasSpecialChar   = false;

  for (char c : lyrics)
  {
    if (c == '[')
    {
      lines.push_back(currentLine);
      currentLine.clear();
      insideBrackets = true;
    }

    if (c == '(' || c == ')' || c == '{' || c == '}')
      wasSpecialChar = !wasSpecialChar;

    if (insideBrackets)
    {
      currentLine += c;
      if (c == ']')
      {
        lines.push_back(currentLine);
        currentLine.clear();
        insideBrackets = false;
      }
    }
    else
    {
      if (std::isupper(c) && !lastWasUppercase && !wasSpecialChar && !currentLine.empty())
      {
        lines.push_back(currentLine); // Start a new line if uppercase
        currentLine.clear();
      }
      currentLine += c;
      lastWasUppercase = std::isupper(c);
    }
  }

  if (!currentLine.empty())
  {
    lines.push_back(currentLine); // Push any remaining text after the loop
  }

  return lines;
}

#endif
