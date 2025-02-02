#pragma once

#include "../arg-handler.hpp"
#include "../dirsort/taglib_parser.h"
#include "./colors.hpp"
#include "./keymaps.hpp"
#include "components/image_view.hpp"
#include <algorithm>
#include <cctype>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

/** STRING TRUNCATION MACROS */
#define MAX_LENGTH_SONG_NAME   50
#define MAX_LENGTH_ARTIST_NAME 30

#define SONG_TITLE_DELIM " • "
#define LYRICS_AVAIL     "L*"
#define ADDN_PROPS_AVAIL "&*"
#define STATUS_BAR_DELIM " | "

auto handleToggleMute(int* volume, int* lastVolume, bool* muted) -> int
{
  *muted = !*muted;
  if (*muted)
  {
    *lastVolume = *volume;
    *volume     = 0;
  }
  else
  {
    *volume = *lastVolume;
  }
  return *volume;
}

auto formatDiscTrackInfo(const int& disc_number, const int& track_number)
{
  std::string formatted_info =
    " " + std::to_string(disc_number) + "-" + std::to_string(track_number) + "  ";

  return formatted_info;
}

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

auto formatAdditionalInfo(const std::string& genre, const bool& has_comment, const bool& has_lyrics,
                          const bool& show_bitrate, const int& bitrate) -> std::string
{
  std::string additional_info = "";
  if (genre != "Unknown Genre")
  {
    additional_info += "Genre: " + genre + STATUS_BAR_DELIM;
  }
  if (has_comment)
  {
    additional_info += ADDN_PROPS_AVAIL;
  }
  if (has_lyrics)
  {
    additional_info += STATUS_BAR_DELIM;
    additional_info += LYRICS_AVAIL;
  }
  if (show_bitrate)
  {
    additional_info += STATUS_BAR_DELIM;
    additional_info += std::to_string(bitrate) + " kbps";
  }
  additional_info += STATUS_BAR_DELIM;

  return additional_info;
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
        return vbox({
                 image_view(thumbnailFilePath) | size(WIDTH, LESS_THAN, 50) |
                   size(HEIGHT, LESS_THAN, 50),
               }) |
               center;
      });

    auto metadataView =
      vbox({
        hbox({text(songTitle) | center | bold | size(WIDTH, EQUAL, MAX_LENGTH_SONG_NAME)}) | center,
        hbox({text(artistName) | dim}) | center,
        hbox({text(albumName) | underlined, text(SONG_TITLE_DELIM),
              text(std::to_string(year)) | dim}) |
          center,
        hbox({text("Track ") | dim, text(std::to_string(trackNumber)) | bold, text(" on Disc "),
              text(std::to_string(discNumber)) | bold}) |
          center,
        separator(),
        hbox({text("Genre: "), text(genre) | bold}) | center,
        hbox({text("Format: "), text(getMimeTypeFromExtension(songFilePath)) | bold}) | center,
      }) |
      borderRounded;

    auto progressBar = vbox({
      separator(),
      hbox({text("Playback Progress") | bold}) | center,
      hbox({gauge(progress) | flex,
            text(" " + std::to_string(static_cast<int>(progress * 100)) + "%")}),
    });

    auto modernUI = vbox({
                      thumbnail->Render() | flex_shrink,
                      separator(),
                      metadataView,
                      separator(),
                      progressBar,
                    }) |
                    borderRounded;

    return modernUI;
  }

  // Fallback UI when thumbnail extraction fails
  auto errorView = vbox({
                     text("⚠ Thumbnail Unavailable") | bold | center | color(Color::Red),
                     separator(),
                     text("Ensure the file has embedded artwork.") | dim | center,
                   }) |
                   border;

  return errorView;
}

auto RenderSearchBar(std::string& user_input) -> Element
{
  return hbox({
           text("/") | color(Color::GrayLight),
           text(user_input) | color(Color::LightSteelBlue) | bold | inverted,
         }) |
         color(Color::GrayDark) | bgcolor(Color::Black) | size(HEIGHT, EQUAL, 1);
}

auto RenderDialog(const std::string& dialog_message) -> Element
{
  return window(text(" File Information ") | bold | center |
                  getTrueColor(TrueColors::Color::White) | getTrueBGColor(TrueColors::Color::Gray),
                vbox({
                  text(dialog_message) | getTrueColor(TrueColors::Color::Coral),
                  separator() | color(Color::GrayLight),
                  text("Press 'x' to close") | dim | center | getTrueColor(TrueColors::Color::Pink),
                }) |
                  center) |
         size(WIDTH, LESS_THAN, 60) | size(HEIGHT, LESS_THAN, 8) |
         getTrueBGColor(TrueColors::Color::Black) | borderHeavy;
}

auto RenderHelpScreen(Keybinds& global_keybinds) -> Element
{
  auto title = text("inLimbo Controls") | bold | getTrueColor(TrueColors::Color::Teal);

  // Helper function to create a single keybind row
  auto createRow =
    [](const std::string& key, const std::string& description, TrueColors::Color color)
  {
    return hbox({
      text(key) | getTrueColor(color),
      text("  --  ") | getTrueColor(TrueColors::Color::Gray),
      text(description) | getTrueColor(TrueColors::Color::White),
    });
  };

  // List of keybinds for easier modification
  std::vector<std::tuple<std::string, std::string, TrueColors::Color>> keybinds = {
    {charToStr(global_keybinds.scroll_up), "Scroll up in the current view",
     TrueColors::Color::LightCyan},
    {charToStr(global_keybinds.scroll_down), "Scroll down in the current view",
     TrueColors::Color::LightCyan},
    {charToStr(global_keybinds.toggle_focus), "Switch focus between panes",
     TrueColors::Color::LightYellow},
    {charToStr(global_keybinds.show_help), "Toggle this help window", TrueColors::Color::Cyan},
    {charToStr(global_keybinds.toggle_play), "Play/Pause playback", TrueColors::Color::Teal},
    {charToStr(global_keybinds.play_song), "Play the selected song", TrueColors::Color::LightGreen},
    {charToStr(global_keybinds.play_song_next), "Skip to next song", TrueColors::Color::LightCyan},
    {charToStr(global_keybinds.play_song_prev), "Go back to previous song",
     TrueColors::Color::LightCyan},
    {charToStr(global_keybinds.vol_up), "Increase volume", TrueColors::Color::LightGreen},
    {charToStr(global_keybinds.vol_down), "Decrease volume", TrueColors::Color::LightGreen},
    {charToStr(global_keybinds.toggle_mute), "Mute/Unmute audio", TrueColors::Color::LightRed},
    {charToStr(global_keybinds.quit_app), "Quit inLimbo", TrueColors::Color::LightRed},
    {charToStr(global_keybinds.seek_ahead_5), "Seek forward by 5 seconds",
     TrueColors::Color::Orange},
    {charToStr(global_keybinds.seek_behind_5), "Seek backward by 5 seconds",
     TrueColors::Color::Orange},
    {charToStr(global_keybinds.view_lyrics), "View lyrics for the current song",
     TrueColors::Color::LightBlue},
    {charToStr(global_keybinds.goto_main_screen), "Return to main UI", TrueColors::Color::Teal},
    {charToStr(global_keybinds.replay_song), "Replay the current song", TrueColors::Color::Orange},
    {charToStr(global_keybinds.add_song_to_queue), "Add selected song to queue",
     TrueColors::Color::LightPink},
    {charToStr(global_keybinds.add_artists_songs_to_queue), "Queue all songs by the artist",
     TrueColors::Color::LightPink},
    {charToStr(global_keybinds.remove_song_from_queue), "Remove selected song from queue",
     TrueColors::Color::Teal},
    {charToStr(global_keybinds.play_this_song_next), "Play selected song next",
     TrueColors::Color::LightMagenta},
    {charToStr(global_keybinds.view_song_queue), "View currently queued songs",
     TrueColors::Color::LightYellow},
    {charToStr(global_keybinds.view_current_song_info), "View info of the currently playing song",
     TrueColors::Color::LightCyan},
    {charToStr(global_keybinds.toggle_audio_devices), "Switch between available audio devices",
     TrueColors::Color::LightBlue},
    {charToStr(global_keybinds.search_menu), "Open the search menu", TrueColors::Color::LightGreen},
    {"gg", "Go to the top of the active menu", TrueColors::Color::LightBlue},
    {"G", "Go to the bottom of the active menu", TrueColors::Color::LightBlue},
    {"x", "Close the dialog box", TrueColors::Color::LightRed},
  };

  // Generate controls list dynamically
  std::vector<Element> control_elements;
  for (const auto& [key, description, color] : keybinds)
  {
    control_elements.push_back(createRow(key, description, color));
  }

  auto controls_list =
    vbox(std::move(control_elements)) | getTrueColor(TrueColors::Color::LightGreen);

  // Symbols Legend
  auto symbols_explanation = vbox({
    hbox({text(LYRICS_AVAIL), text(" -> "), text("The current song has lyrics metadata.")}) |
      getTrueColor(TrueColors::Color::LightCyan),
    hbox({text(ADDN_PROPS_AVAIL), text("  -> "),
          text("The current song has additional properties metadata.")}) |
      getTrueColor(TrueColors::Color::LightYellow),
  });

  // Application details
  auto app_name = text("inLimbo - Music player that keeps you in Limbo...") | bold |
                  getTrueColor(TrueColors::Color::Teal);
  auto contributor =
    text("Developed by: Siddharth Karanam (nots1dd)") | getTrueColor(TrueColors::Color::LightGreen);
  auto github_link = hbox({
    text("GitHub: "),
    text("Click me!") | underlined | bold | getTrueColor(TrueColors::Color::LightBlue) |
      hyperlink(REPOSITORY_URL),
  });

  std::string footer_text =
    "Press '" + charToStr(global_keybinds.show_help) + "' to return to inLimbo.";
  auto footer = vbox({
                  app_name,
                  contributor,
                  github_link,
                  text(footer_text) | getTrueColor(TrueColors::Color::LightYellow) | center,
                }) |
                flex | border;

  return vbox({
           title,
           controls_list | border | flex,
           text("Symbols Legend") | bold | getTrueColor(TrueColors::Color::LightBlue),
           symbols_explanation | border | flex,
           footer,
         }) |
         flex;
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

auto RenderStatusBar(const std::string& status, const std::string& current_song_info,
                     const std::string& additional_info, const std::string& year_info,
                     InLimboColors& global_colors, const std::string& current_artist) -> Element
{
  return hbox({
           text(status) | getTrueColor(TrueColors::Color::Black) | bold,
           text(current_artist) | color(global_colors.status_bar_artist_col) | bold |
             size(WIDTH, LESS_THAN, MAX_LENGTH_ARTIST_NAME),
           text(current_song_info) | bold | color(global_colors.status_bar_song_col) |
             size(WIDTH, LESS_THAN, MAX_LENGTH_SONG_NAME),
           filler(), // Push the right-aligned content to the end
           hbox({
             text(additional_info) | bold | color(global_colors.status_bar_addn_info_col) | flex,
             text(year_info) | color(global_colors.status_bar_addn_info_col) |
               size(WIDTH, LESS_THAN, 15),
             text(" "),
           }) |
             align_right,
         }) |
         size(HEIGHT, EQUAL, 1) | bgcolor(global_colors.status_bar_bg);
}

auto RenderVolumeBar(int volume, ftxui::Color volume_bar_col) -> Element
{
  return hbox({
    text(" Vol: ") | dim,
    gauge(volume / 100.0) | size(WIDTH, EQUAL, 10) | color(volume_bar_col),
    text(std::to_string(volume) + "%") | dim,
  });
}
