#ifndef MISC_HEADER
#define MISC_HEADER

#include "../dirsort/taglib_parser.h"
#include "./colors.hpp"
#include "components/image_view.hpp"
#include <algorithm>
#include <cctype>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

/** STRING TRUNCATION MACROS */
#define MAX_LENGTH_SONG_NAME   50
#define MAX_LENGTH_ARTIST_NAME 30

struct ComponentState
{
  Component artists_list;
  Component songs_list;
  Component songs_queue_comp;
  Component lyrics_scroller;
  Component MainRenderer;
  Component ThumbnailRenderer;
  Component audioDeviceMenu;
};

struct PlayingState
{
  std::string                                  artist;
  std::string                                  title;
  std::string                                  genre;
  std::string                                  album;
  bool                                         has_comment = false;
  bool                                         has_lyrics  = false;
  int                                          duration;
  int                                          bitrate;
  unsigned int                                 year       = 0;
  unsigned int                                 track      = 0;
  unsigned int                                 discNumber = 0;
  std::string                                  lyrics;
  std::string                                  comment;
  std::unordered_map<std::string, std::string> additionalProperties;
  std::string                                  filePath;
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

auto charToStr(char ch) -> std::string
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

auto renderAlbumName(const std::string& album_name, const int& year, ftxui::Color sel_color,
                     ftxui::Color sel_color_fg)
{
  auto albumText = vbox(text(album_name) | color(sel_color_fg) | bold);
  return hbox({text(" "), albumText, filler(),
               text(std::to_string(year)) | color(sel_color_fg) | bold | align_right, text(" ")}) |
         bgcolor(sel_color);
}

auto renderSongName(const std::string& disc_track_info, const std::string& song_name,
                    const int& duration)
{
  return hbox({text(disc_track_info), text(song_name) | bold | flex_grow,
               filler(), // Spacer for dynamic layout
               text(FormatTime(duration)) | align_right});
}

auto RenderArtistNames(const std::vector<std::string>& artist_list)
{
  std::vector<Element> artist_names_elements;

  for (const auto& a : artist_list)
  {
    auto artistElement = hbox({text(" "), text(a) | bold});
    artist_names_elements.push_back(artistElement);
  }

  return artist_names_elements;
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
  for (const auto& item : items)
  {
    rendered_items.push_back(item | frame);
  }

  return vbox(std::move(rendered_items));
}

auto RenderArtistMenu(const std::vector<std::string>& artist_list)
{
  auto     items = RenderArtistNames(artist_list);
  Elements rendered_items;
  for (const auto& item : items)
  {
    rendered_items.push_back(item | frame);
  }

  return vbox(std::move(rendered_items));
}

auto getMimeTypeFromExtension(const std::string& filePath) -> std::string
{
  // Map file extensions to MIME types
  static std::map<std::string, std::string> mimeTypes = {
    {".mp3", "audio/mpeg"},  {".flac", "audio/flac"},    {".wav", "audio/wav"},
    {".ogg", "audio/ogg"},   {".aac", "audio/aac"},      {".m4a", "audio/mp4"},
    {".opus", "audio/opus"}, {".wma", "audio/x-ms-wma"}, {".alac", "audio/alac"}};

  // Extract the file extension
  std::string::size_type idx = filePath.rfind('.');
  if (idx != std::string::npos)
  {
    std::string extension = filePath.substr(idx);
    auto        it        = mimeTypes.find(extension);
    if (it != mimeTypes.end())
    {
      return it->second;
    }
  }

  // Default MIME type if the extension is unknown
  return "application/octet-stream";
}

auto RenderThumbnail(const std::string& songFilePath, const std::string& cacheDirPath,
                     const std::string& songTitle, const std::string& artistName,
                     const std::string& albumName, const std::string& genre, unsigned int year,
                     unsigned int trackNumber, unsigned int discNumber,
                     float progress) // progress: a value between 0.0 (0%) and 1.0 (100%)
{
  auto thumbnailFilePath = cacheDirPath + "thumbnail.png";

  if (extractThumbnail(songFilePath, thumbnailFilePath))
  {
    auto thumbnail = Renderer(
      [&]
      {
        return vbox({image_view(thumbnailFilePath)}) |
               center; // [TODO] Is not being centered properly
      });

    auto metadataView = vbox({
      hbox({text(albumName) | bold | underlined}) | center,
      hbox({text(songTitle) | bold, text(" by "), text(artistName) | bold, text(" ["),
            text(std::to_string(year)), text("]"), text(" ("),
            text(std::to_string(discNumber)) | bold, text("/"),
            text(std::to_string(trackNumber)) | bold, text(") -- {"),
            text(getMimeTypeFromExtension(songFilePath)) | bold, text("}")}) |
        center,
      hbox({text("Seems like a "), text(genre) | bold, text(" type of song...")}) | center, // Genre
    });

    auto progressBar = hbox({
      text("Progress: ") | bold,
      gauge(progress) | flex, // Progress bar
      text(" "),
      text(std::to_string(static_cast<int>(progress * 100)) + "%"),
    });

    auto thumbNailEle = vbox({thumbnail->Render()});

    auto modernUI = vbox({
                      thumbNailEle | flex_shrink, // Centered and scaled thumbnail
                      separator(),
                      metadataView | borderRounded, // Metadata in a rounded bordered box
                      separator(),
                      progressBar, // Progress bar below the metadata
                    }) |
                    borderRounded;

    return modernUI;
  }

  // Fallback for when thumbnail extraction fails
  auto errorView = vbox({
    text("Error: Thumbnail not found!") | center | dim,
    separator(),
    text("Please ensure the file has embedded artwork.") | center,
  });

  return errorView;
}

void searchModeIndices(const std::vector<std::string>& words, const std::string& prefix,
                       std::vector<int>& search_indices)
{
  auto start = lower_bound(words.begin(), words.end(), prefix);

  for (auto it = start; it != words.end(); ++it)
  {
    if (it->substr(0, prefix.size()) != prefix)
      break;
    search_indices.push_back(std::distance(words.begin(), it));
  }
}

void UpdateSelectedIndex(int& index, int max_size, bool move_down)
{
  if (max_size == 0)
    return;
  index = move_down ? (index + 1) % max_size : (index == 0 ? max_size - 1 : index - 1);
}

#endif
