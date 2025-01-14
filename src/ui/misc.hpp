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

struct ComponentState
{
  Component artists_list;
  Component songs_list;
  Component songs_queue_comp;
  Component lyrics_scroller;
  Component MainRenderer;
};

auto formatLyrics(const std::string& lyrics)
{
  std::vector<std::string> lines;
  std::string              currentLine;
  bool                     insideSquareBrackets = false;
  bool                     insideCurlBrackets   = false;
  bool                     lastWasUppercase     = false;
  bool                     lastWasSpecialChar   = false; // Tracks special characters within words
  char                     previousChar         = '\0';

  for (char c : lyrics)
  {
    if (c == '[' || c == '(')
    {
      if (!currentLine.empty())
      {
        lines.push_back(currentLine);
        currentLine.clear();
      }
      if (c == '[')
        insideSquareBrackets = true;
      else
        insideCurlBrackets = true;
      currentLine += c;
      continue;
    }

    if (insideSquareBrackets || insideCurlBrackets)
    {
      currentLine += c;
      if (c == ']' && insideSquareBrackets)
      {
        lines.push_back(currentLine);
        currentLine.clear();
        insideSquareBrackets = false;
      }

      else if (c == ')' && insideCurlBrackets)
      {
        lines.push_back(currentLine);
        currentLine.clear();
        insideCurlBrackets = false;
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

auto FormatTime(int seconds)
{
  int minutes = seconds / 60;
  seconds     = seconds % 60;
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2)
     << seconds << " ";
  return ss.str();
}

auto getTrueColor(TrueColors::Color color) { return ftxui::color(TrueColors::GetColor(color)); }

auto getTrueBGColor(TrueColors::Color color) { return ftxui::bgcolor(TrueColors::GetColor(color)); }

auto renderAlbumName(const std::string& album_name, const int& year, ftxui::Color sel_color)
{
  return hbox({text(" "), text(album_name) | bold, filler(),
               text(std::to_string(year)) | dim | align_right, text(" ")}) |
         inverted | color(sel_color) | dim;
}

auto renderSongName(const std::string& disc_track_info, const std::string& song_name,
                    const int& duration)
{
  return hbox({text(disc_track_info) | dim, text(song_name) | bold | flex_grow,
               filler(), // Spacer for dynamic layout
               text(FormatTime(duration)) | align_right});
}

auto CreateMenu(const std::vector<std::string>* vecLines, int* currLine)
{
  MenuOption menu_options;
  menu_options.on_change     = [&]() {};
  menu_options.focused_entry = currLine;

  return Menu(vecLines, currLine, menu_options);
}

auto RenderSongMenu(const std::vector<Element>& items)
{
  Elements rendered_items;
  for (int i = 0; i < items.size(); ++i)
  {
    rendered_items.push_back(items[i] | frame);
  }

  return vbox(std::move(rendered_items));
}

#endif
