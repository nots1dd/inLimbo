#ifndef COLORS_HPP
#define COLORS_HPP

#include <cstdint> // For uint8_t
#include <ftxui/screen/color.hpp>

using namespace ftxui;

namespace TrueColors
{

// Enum class for predefined true colors
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

// Function to map the enum to ftxui::Color::RGB
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

struct InLimboColors
{
  TrueColors::Color active_win_color;
  TrueColors::Color album_name_bg;
  TrueColors::Color menu_cursor_bg;
};

// Function to map color strings from TOML to the enum
TrueColors::Color parseColor(const std::string& color_name)
{
  static const std::unordered_map<std::string, TrueColors::Color> color_map = {
    {"Black", TrueColors::Color::Black},
    {"White", TrueColors::Color::White},
    {"Red", TrueColors::Color::Red},
    {"LightRed", TrueColors::Color::LightRed},
    {"Green", TrueColors::Color::Green},
    {"LightGreen", TrueColors::Color::LightGreen},
    {"Blue", TrueColors::Color::Blue},
    {"LightBlue", TrueColors::Color::LightBlue},
    {"Yellow", TrueColors::Color::Yellow},
    {"LightYellow", TrueColors::Color::LightYellow},
    {"Cyan", TrueColors::Color::Cyan},
    {"LightCyan", TrueColors::Color::LightCyan},
    {"Magenta", TrueColors::Color::Magenta},
    {"LightMagenta", TrueColors::Color::LightMagenta},
    {"Gray", TrueColors::Color::Gray},
    {"LightGray", TrueColors::Color::LightGray},
    {"DarkGray", TrueColors::Color::DarkGray},
    {"Orange", TrueColors::Color::Orange},
    {"LightOrange", TrueColors::Color::LightOrange},
    {"Purple", TrueColors::Color::Purple},
    {"LightPurple", TrueColors::Color::LightPurple},
    {"Pink", TrueColors::Color::Pink},
    {"LightPink", TrueColors::Color::LightPink},
    {"Teal", TrueColors::Color::Teal},
    {"LightTeal", TrueColors::Color::LightTeal},
    {"SkyBlue", TrueColors::Color::SkyBlue},
    {"Coral", TrueColors::Color::Coral},
    {"Lime", TrueColors::Color::Lime},
    {"Lavender", TrueColors::Color::Lavender},
    {"Crimson", TrueColors::Color::Crimson},
    {"Gold", TrueColors::Color::Gold},
    {"Indigo", TrueColors::Color::Indigo},
    {"Mint", TrueColors::Color::Mint},
    {"Navy", TrueColors::Color::Navy},
    {"Peach", TrueColors::Color::Peach},
    {"Sand", TrueColors::Color::Sand},
    {"SeaGreen", TrueColors::Color::SeaGreen},
    {"LightSeaGreen", TrueColors::Color::LightSeaGreen},
    {"SlateBlue", TrueColors::Color::SlateBlue},
    {"LightSlateBlue", TrueColors::Color::LightSlateBlue},
    {"SunsetOrange", TrueColors::Color::SunsetOrange},
    {"Turquoise", TrueColors::Color::Turquoise},
    {"LightTurquoise", TrueColors::Color::LightTurquoise}};

  auto it = color_map.find(color_name);
  if (it != color_map.end())
  {
    return it->second;
  }

  std::cerr << "Error: Unsupported or empty color '" << color_name
            << "'. Please check your configuration." << std::endl;
  std::exit(EXIT_FAILURE);
}

// Function to parse the colors from the TOML file into the InLimboColors struct
InLimboColors parseColors()
{
  InLimboColors colors;

  // Mapping of fields in the InLimboColors struct
  const std::unordered_map<std::string, TrueColors::Color*> field_map = {
    {"active_win_color", &colors.active_win_color},
    {"album_name_bg", &colors.album_name_bg},
    {"menu_cursor_bg", &colors.menu_cursor_bg}};

  for (const auto& [field, member_color] : field_map)
  {
    std::string color_name = std::string(parseTOMLField(PARENT_COLORS, field));
    *member_color          = parseColor(color_name);
  }

  return colors;
}

#endif
