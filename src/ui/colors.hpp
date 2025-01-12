#ifndef COLORS_HPP
#define COLORS_HPP

#include <cstdint> // For uint8_t
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>

using namespace ftxui;

namespace TrueColors
{

/**
 * @enum Color
 * @brief Enumeration for predefined true colors.
 * 
 * This enumeration provides a set of predefined colors that can be used for
 * setting terminal colors in the application. Each color maps to an RGB value.
 */

enum class Color
{
  Black,
  White,
  Red,
  LightRed,
  Green,
  LightGreen,
  Blue,
  LightBlue,
  Yellow,
  LightYellow,
  Cyan,
  LightCyan,
  Magenta,
  LightMagenta,
  Gray,
  LightGray,
  DarkGray,
  Orange,
  LightOrange,
  Purple,
  LightPurple,
  Pink,
  LightPink,
  Teal,
  LightTeal,
  SkyBlue,
  Coral,
  Lime,
  Lavender,
  Crimson,
  Gold,
  Indigo,
  Mint,
  Navy,
  Peach,
  Sand,
  SeaGreen,
  LightSeaGreen,
  SlateBlue,
  LightSlateBlue,
  SunsetOrange,
  Turquoise,
  LightTurquoise
};

/**
 * @brief Maps a predefined color enum to its corresponding ftxui::Color::RGB value.
 * 
 * @param color The predefined color enum.
 * @return ftxui::Color The corresponding RGB color.
 */
ftxui::Color GetColor(Color color)
{
  switch (color)
  {
    case Color::Black:
      return ftxui::Color::RGB(0, 0, 0);
    case Color::White:
      return ftxui::Color::RGB(255, 255, 255);
    case Color::Red:
      return ftxui::Color::RGB(255, 0, 0);
    case Color::LightRed:
      return ftxui::Color::RGB(255, 102, 102);
    case Color::Green:
      return ftxui::Color::RGB(0, 255, 0);
    case Color::LightGreen:
      return ftxui::Color::RGB(144, 238, 144);
    case Color::Blue:
      return ftxui::Color::RGB(0, 0, 255);
    case Color::LightBlue:
      return ftxui::Color::RGB(173, 216, 230);
    case Color::Yellow:
      return ftxui::Color::RGB(255, 255, 0);
    case Color::LightYellow:
      return ftxui::Color::RGB(255, 255, 153);
    case Color::Cyan:
      return ftxui::Color::RGB(0, 255, 255);
    case Color::LightCyan:
      return ftxui::Color::RGB(224, 255, 255);
    case Color::Magenta:
      return ftxui::Color::RGB(255, 0, 255);
    case Color::LightMagenta:
      return ftxui::Color::RGB(255, 153, 255);
    case Color::Gray:
      return ftxui::Color::RGB(128, 128, 128);
    case Color::LightGray:
      return ftxui::Color::RGB(211, 211, 211);
    case Color::DarkGray:
      return ftxui::Color::RGB(64, 64, 64);
    case Color::Orange:
      return ftxui::Color::RGB(255, 165, 0);
    case Color::LightOrange:
      return ftxui::Color::RGB(255, 200, 124);
    case Color::Purple:
      return ftxui::Color::RGB(128, 0, 128);
    case Color::LightPurple:
      return ftxui::Color::RGB(216, 191, 216);
    case Color::Pink:
      return ftxui::Color::RGB(255, 192, 203);
    case Color::LightPink:
      return ftxui::Color::RGB(255, 182, 193);
    case Color::Teal:
      return ftxui::Color::RGB(0, 128, 128);
    case Color::LightTeal:
      return ftxui::Color::RGB(144, 224, 224);
    case Color::SkyBlue:
      return ftxui::Color::RGB(135, 206, 235);
    case Color::Coral:
      return ftxui::Color::RGB(255, 127, 80);
    case Color::Lime:
      return ftxui::Color::RGB(191, 255, 0);
    case Color::Lavender:
      return ftxui::Color::RGB(230, 230, 250);
    case Color::Crimson:
      return ftxui::Color::RGB(220, 20, 60);
    case Color::Gold:
      return ftxui::Color::RGB(255, 215, 0);
    case Color::Indigo:
      return ftxui::Color::RGB(75, 0, 130);
    case Color::Mint:
      return ftxui::Color::RGB(152, 255, 152);
    case Color::Navy:
      return ftxui::Color::RGB(0, 0, 128);
    case Color::Peach:
      return ftxui::Color::RGB(255, 218, 185);
    case Color::Sand:
      return ftxui::Color::RGB(244, 164, 96);
    case Color::SeaGreen:
      return ftxui::Color::RGB(46, 139, 87);
    case Color::LightSeaGreen:
      return ftxui::Color::RGB(152, 255, 204);
    case Color::SlateBlue:
      return ftxui::Color::RGB(106, 90, 205);
    case Color::LightSlateBlue:
      return ftxui::Color::RGB(176, 196, 222);
    case Color::SunsetOrange:
      return ftxui::Color::RGB(255, 99, 71);
    case Color::Turquoise:
      return ftxui::Color::RGB(64, 224, 208);
    case Color::LightTurquoise:
      return ftxui::Color::RGB(175, 238, 238);
  }
  return ftxui::Color::RGB(0, 0, 0); // Default to black
}

} // namespace TrueColors

/**
 * @struct InLimboColors
 * @brief Represents a collection of colors used in the application.
 * 
 * This structure holds various color fields that can be customized
 * through configurations, such as active window color, background colors,
 * and title colors.
 */

struct InLimboColors
{
  ftxui::Color active_win_border_color;
  ftxui::Color inactive_win_border_color;
  ftxui::Color album_name_bg;
  ftxui::Color menu_cursor_bg;
  ftxui::Color artists_title_bg;
  ftxui::Color artists_title_fg;
  ftxui::Color songs_title_bg;
  ftxui::Color songs_title_fg;
  ftxui::Color song_queue_menu_bor_col;
  ftxui::Color song_queue_menu_fg;
  ftxui::Color progress_bar_playing_col;
  ftxui::Color progress_bar_not_playing_col;
  ftxui::Color volume_bar_col;
  ftxui::Color status_bar_bg;
  ftxui::Color status_bar_artist_col;
  ftxui::Color status_bar_song_col;
};

/**
 * @brief Parses a hexadecimal color string into an ftxui::Color::RGB value.
 * 
 * @param hex The hexadecimal color string in the format `#RRGGBB`.
 * @return ftxui::Color The corresponding RGB color.
 * @throws std::exit on invalid format.
 */

ftxui::Color parseHexColor(const std::string& hex)
{
  // Validate hex format
  std::regex  hex_regex("^#([0-9a-fA-F]{6})$"); // Matches #RRGGBB
  std::smatch match;

  if (!std::regex_match(hex, match, hex_regex))
  {
    std::cerr << "Error: Invalid hexadecimal color format: '" << hex
              << "'. Use a string like `#RRGGBB`." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Extract RGB components from the validated hex string
  int r = std::stoi(hex.substr(1, 2), nullptr, 16);
  int g = std::stoi(hex.substr(3, 2), nullptr, 16);
  int b = std::stoi(hex.substr(5, 2), nullptr, 16);

  return ftxui::Color::RGB(r, g, b);
}

/**
 * @brief Parses a color name or hex value into an ftxui::Color.
 * 
 * @param color_name The name or hexadecimal representation of the color.
 * @return ftxui::Color The corresponding RGB color.
 * @throws std::exit on invalid or unsupported colors.
 */
ftxui::Color parseColor(const std::string& color_name)
{
  if (color_name[0] == '#')
  {
    return parseHexColor(color_name);
  }

  static const std::unordered_map<std::string, ftxui::Color> color_map = {
    {"Black", TrueColors::GetColor(TrueColors::Color::Black)},
    {"White", TrueColors::GetColor(TrueColors::Color::White)},
    {"Red", TrueColors::GetColor(TrueColors::Color::Red)},
    {"LightRed", TrueColors::GetColor(TrueColors::Color::LightRed)},
    {"Green", TrueColors::GetColor(TrueColors::Color::Green)},
    {"LightGreen", TrueColors::GetColor(TrueColors::Color::LightGreen)},
    {"Blue", TrueColors::GetColor(TrueColors::Color::Blue)},
    {"LightBlue", TrueColors::GetColor(TrueColors::Color::LightBlue)},
    {"Yellow", TrueColors::GetColor(TrueColors::Color::Yellow)},
    {"LightYellow", TrueColors::GetColor(TrueColors::Color::LightYellow)},
    {"Cyan", TrueColors::GetColor(TrueColors::Color::Cyan)},
    {"LightCyan", TrueColors::GetColor(TrueColors::Color::LightCyan)},
    {"Magenta", TrueColors::GetColor(TrueColors::Color::Magenta)},
    {"LightMagenta", TrueColors::GetColor(TrueColors::Color::LightMagenta)},
    {"Gray", TrueColors::GetColor(TrueColors::Color::Gray)},
    {"LightGray", TrueColors::GetColor(TrueColors::Color::LightGray)},
    {"DarkGray", TrueColors::GetColor(TrueColors::Color::DarkGray)},
    {"Orange", TrueColors::GetColor(TrueColors::Color::Orange)},
    {"LightOrange", TrueColors::GetColor(TrueColors::Color::LightOrange)},
    {"Purple", TrueColors::GetColor(TrueColors::Color::Purple)},
    {"LightPurple", TrueColors::GetColor(TrueColors::Color::LightPurple)},
    {"Pink", TrueColors::GetColor(TrueColors::Color::Pink)},
    {"LightPink", TrueColors::GetColor(TrueColors::Color::LightPink)},
    {"Teal", TrueColors::GetColor(TrueColors::Color::Teal)},
    {"LightTeal", TrueColors::GetColor(TrueColors::Color::LightTeal)},
    {"SkyBlue", TrueColors::GetColor(TrueColors::Color::SkyBlue)},
    {"Coral", TrueColors::GetColor(TrueColors::Color::Coral)},
    {"Lime", TrueColors::GetColor(TrueColors::Color::Lime)},
    {"Lavender", TrueColors::GetColor(TrueColors::Color::Lavender)},
    {"Crimson", TrueColors::GetColor(TrueColors::Color::Crimson)},
    {"Gold", TrueColors::GetColor(TrueColors::Color::Gold)},
    {"Indigo", TrueColors::GetColor(TrueColors::Color::Indigo)},
    {"Mint", TrueColors::GetColor(TrueColors::Color::Mint)},
    {"Navy", TrueColors::GetColor(TrueColors::Color::Navy)},
    {"Peach", TrueColors::GetColor(TrueColors::Color::Peach)},
    {"Sand", TrueColors::GetColor(TrueColors::Color::Sand)},
    {"SeaGreen", TrueColors::GetColor(TrueColors::Color::SeaGreen)},
    {"LightSeaGreen", TrueColors::GetColor(TrueColors::Color::LightSeaGreen)},
    {"SlateBlue", TrueColors::GetColor(TrueColors::Color::SlateBlue)},
    {"LightSlateBlue", TrueColors::GetColor(TrueColors::Color::LightSlateBlue)},
    {"SunsetOrange", TrueColors::GetColor(TrueColors::Color::SunsetOrange)},
    {"Turquoise", TrueColors::GetColor(TrueColors::Color::Turquoise)},
    {"LightTurquoise", TrueColors::GetColor(TrueColors::Color::LightTurquoise)}};

  auto it = color_map.find(color_name);
  if (it != color_map.end())
  {
    return it->second;
  }

  std::cerr << "Error: Unsupported or empty color '" << color_name
            << "'. Please check your configuration." << std::endl;
  std::exit(EXIT_FAILURE);
}

/**
 * @brief Parses colors from a TOML configuration into an InLimboColors struct.
 * 
 * @return InLimboColors The populated color configuration.
 */
InLimboColors parseColors()
{
  InLimboColors colors;

  // Mapping of fields in the InLimboColors struct
  const std::unordered_map<std::string, ftxui::Color*> field_map = {
    {"active_win_border_color", &colors.active_win_border_color},
    {"inactive_win_border_color", &colors.inactive_win_border_color},
    {"album_name_bg", &colors.album_name_bg},
    {"menu_cursor_bg", &colors.menu_cursor_bg},
    {"artists_title_bg", &colors.artists_title_bg},
    {"artists_title_fg", &colors.artists_title_fg},
    {"songs_title_bg", &colors.songs_title_bg},
    {"songs_title_fg", &colors.songs_title_fg},
    {"song_queue_menu_bor_col", &colors.song_queue_menu_bor_col},
    {"song_queue_menu_fg", &colors.song_queue_menu_fg},
    {"progress_bar_playing_col", &colors.progress_bar_playing_col},
    {"progress_bar_not_playing_col", &colors.progress_bar_not_playing_col},
    {"volume_bar_col", &colors.volume_bar_col},
    {"status_bar_bg", &colors.status_bar_bg},
    {"status_bar_artist_col", &colors.status_bar_artist_col},
    {"status_bar_song_col", &colors.status_bar_song_col},

  };

  for (const auto& [field, member_color] : field_map)
  {
    std::string color_name = std::string(parseTOMLField(PARENT_COLORS, field));
    *member_color          = parseColor(color_name);
  }

  return colors;
}

#endif
