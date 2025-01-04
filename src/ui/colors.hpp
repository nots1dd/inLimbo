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

#endif
