#ifndef MISC_HEADER
#define MISC_HEADER

#include "./colors.hpp"
#include <cctype>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
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

std::string charToStr(char ch)
{
  switch (ch)
  {
    case '\t':
      return "Tab";
    case ' ':
      return "Space";
    case '\n':
      return "Enter";
    case 27:
      return "Escape"; // ASCII value for the Escape key
    default:
      return std::string(1, static_cast<char>(ch));
  }
}

std::string FormatTime(int seconds)
{
  int minutes = seconds / 60;
  seconds     = seconds % 60;
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2)
     << seconds << " ";
  return ss.str();
}

// [TODO]: Make the song formatting better
std::string format_song_info(const auto& disc_number, const auto& track_number,
                             const std::string& song_name,
                             const int          duration) // this formatting is pretty bad
{
  return std::to_string(disc_number) + "-" + std::to_string(track_number) + ": " + song_name +
         " ( " + FormatTime(duration) + ")";
}

ftxui::Decorator getTrueColor(TrueColors::Color color)
{
  return ftxui::color(TrueColors::GetColor(color));
}

ftxui::Decorator getTrueBGColor(TrueColors::Color color)
{
  return ftxui::bgcolor(TrueColors::GetColor(color));
}

#endif
