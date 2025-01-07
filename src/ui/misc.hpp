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
  bool                     insideBrackets     = false;
  bool                     lastWasUppercase   = false;
  bool                     lastWasSpecialChar = false; // Tracks special characters within words
  char                     previousChar       = '\0';

  for (char c : lyrics)
  {
    if (c == '[')
    {
      if (!currentLine.empty())
      {
        lines.push_back(currentLine);
        currentLine.clear();
      }
      insideBrackets = true;
      currentLine += c;
      continue;
    }

    if (insideBrackets)
    {
      currentLine += c;
      if (c == ']')
      {
        lines.push_back(currentLine);
        currentLine.clear();
        insideBrackets = false;
      }
      continue;
    }

    if (c == '\'' || c == '-')
    {
      currentLine += c;
      lastWasSpecialChar = true;
      continue;
    }

    if (std::isupper(c) && !lastWasUppercase && !lastWasSpecialChar && !currentLine.empty() &&
        previousChar != '\n' && previousChar != ' ')
    {
      lines.push_back(currentLine);
      currentLine.clear();
    }

    currentLine += c;

    if (c == '\n')
    {
      if (!currentLine.empty())
      {
        lines.push_back(currentLine);
        currentLine.clear();
      }
    }

    lastWasUppercase   = std::isupper(c);
    lastWasSpecialChar = false;
    previousChar       = c;
  }

  if (!currentLine.empty())
  {
    lines.push_back(currentLine);
  }

  // Trim empty lines (optional)
  lines.erase(std::remove_if(lines.begin(), lines.end(),
                             [](const std::string& line) { return line.empty(); }),
              lines.end());

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

ftxui::Decorator getTrueColor(TrueColors::Color color)
{
  return ftxui::color(TrueColors::GetColor(color));
}

ftxui::Decorator getTrueBGColor(TrueColors::Color color)
{
  return ftxui::bgcolor(TrueColors::GetColor(color));
}

Element renderAlbumName(const std::string& album_name, const int& year, TrueColors::Color color)
{
  return hbox({text(" "), text(album_name) | bold, filler(),
               text(std::to_string(year)) | dim | align_right, text(" ")}) |
         inverted | getTrueColor(color) | dim;
}

Element renderSongName(const std::string& disc_track_info, const std::string& song_name,
                       const int& duration)
{
  return hbox({text(disc_track_info) | dim, text(song_name) | bold | flex_grow,
               filler(), // Spacer for dynamic layout
               text(FormatTime(duration)) | align_right});
}

// TODO: Make Song Menu dynamically scrollable
auto RenderSongMenu(const std::vector<Element>& items, int* selected_index, TrueColors::Color color)
{
  Elements rendered_items;
  for (size_t i = 0; i < items.size(); ++i)
  {
    bool is_selected = (i == *selected_index);
    /*bool is_playing     = (i == *playing_index);*/
    auto style          = is_selected ? getTrueColor(color) : nothing;
    auto inverted_style = is_selected ? inverted : nothing;
    /*auto playing_style = is_playing ? getTrueColor(playing_color) : nothing;*/
    rendered_items.push_back(items[i] | style | inverted_style);
  }

  return vbox(std::move(rendered_items));
}

#endif
