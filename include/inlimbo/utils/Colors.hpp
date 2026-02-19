#pragma once

#include "InLimbo-Types.hpp"
#include <array>
#include <optional>
#include <string_view>

namespace utils::colors
{

enum class Basic16 : ui8
{
  Black = 0,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,

  BrightBlack,
  BrightRed,
  BrightGreen,
  BrightYellow,
  BrightBlue,
  BrightMagenta,
  BrightCyan,
  BrightWhite
};

struct RGBA
{
  ui8 r{0};
  ui8 g{0};
  ui8 b{0};
  ui8 a{255};

  constexpr RGBA() = default;
  constexpr RGBA(ui8 rr, ui8 gg, ui8 bb, ui8 aa = 255) : r(rr), g(gg), b(bb), a(aa) {}
};

constexpr auto rgba(ui8 r, ui8 g, ui8 b, ui8 a = 255) -> RGBA { return RGBA{r, g, b, a}; }

// "#RRGGBB" or "#RRGGBBAA"
constexpr auto hexNibble(char c) -> int
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  return -1;
}

constexpr auto hexByte(char a, char b) -> int
{
  const int hi = hexNibble(a);
  const int lo = hexNibble(b);
  if (hi < 0 || lo < 0)
    return -1;
  return (hi << 4) | lo;
}

constexpr auto fromHex(std::string_view hex, RGBA fallback = RGBA{}) -> RGBA
{
  if (hex.empty())
    return fallback;

  size_t i = 0;
  if (hex[0] == '#')
    i = 1;

  const size_t n = hex.size() - i;
  if (!(n == 6 || n == 8))
    return fallback;

  const int r = hexByte(hex[i + 0], hex[i + 1]);
  const int g = hexByte(hex[i + 2], hex[i + 3]);
  const int b = hexByte(hex[i + 4], hex[i + 5]);
  if (r < 0 || g < 0 || b < 0)
    return fallback;

  int a = 255;
  if (n == 8)
  {
    const int aa = hexByte(hex[i + 6], hex[i + 7]);
    a            = (aa < 0) ? 255 : aa;
  }

  return RGBA{(ui8)r, (ui8)g, (ui8)b, (ui8)a};
}

inline auto parseRGBA(std::string_view s) -> std::optional<RGBA>
{
  // ---------- #RRGGBB or #RRGGBBAA ----------
  if (s.size() == 7 || s.size() == 9)
  {
    if (s[0] != '#')
      return std::nullopt;

    int r = hexByte(s[1], s[2]);
    int g = hexByte(s[3], s[4]);
    int b = hexByte(s[5], s[6]);
    if (r < 0 || g < 0 || b < 0)
      return std::nullopt;

    int a = 255;
    if (s.size() == 9)
    {
      a = hexByte(s[7], s[8]);
      if (a < 0)
        return std::nullopt;
    }

    return RGBA{(ui8)r, (ui8)g, (ui8)b, (ui8)a};
  }

  // ---------- r,g,b[,a] ----------
  std::array<int, 4> vals      = {0, 0, 0, 255};
  int                idx       = 0;
  int                cur       = 0;
  bool               haveDigit = false;

  for (char c : s)
  {
    if (c >= '0' && c <= '9')
    {
      haveDigit = true;
      cur       = cur * 10 + (c - '0');
      if (cur > 255)
        return std::nullopt;
    }
    else if (c == ',' && haveDigit)
    {
      if (idx >= 4)
        return std::nullopt;
      vals[idx++] = cur;
      cur         = 0;
      haveDigit   = false;
    }
    else
    {
      return std::nullopt;
    }
  }

  if (!haveDigit || idx < 2 || idx > 3)
    return std::nullopt;

  vals[idx] = cur;

  return RGBA{(ui8)vals[0], (ui8)vals[1], (ui8)vals[2], (ui8)vals[3]};
}

struct Ansi
{
  // Basic ANSI "controls"
  static constexpr std::string_view Esc       = "\x1b[";
  static constexpr std::string_view Clear     = "\x1b[2J\x1b[H";
  static constexpr std::string_view ClearLine = "\x1b[2K";
  static constexpr std::string_view Reset     = "\x1b[0m";
  static constexpr std::string_view Bold      = "\x1b[1m";
  static constexpr std::string_view Dim       = "\x1b[2m";
  static constexpr std::string_view Underline = "\x1b[4m";
  static constexpr std::string_view Italic    = "\x1b[3m";

  static constexpr std::string_view NoBold      = "\x1b[22m";
  static constexpr std::string_view NoUnderline = "\x1b[24m";
  static constexpr std::string_view NoItalic    = "\x1b[23m";

  // ----- 24-bit True Color -----
  static inline auto fgTrue(utils::colors::RGBA c) -> std::string
  {
    return "\x1b[38;2;" + std::to_string((int)c.r) + ";" + std::to_string((int)c.g) + ";" +
           std::to_string((int)c.b) + "m";
  }

  static inline auto bgTrue(utils::colors::RGBA c) -> std::string
  {
    return "\x1b[48;2;" + std::to_string((int)c.r) + ";" + std::to_string((int)c.g) + ";" +
           std::to_string((int)c.b) + "m";
  }

  // ----- 256-color palette -----
  static inline auto fg256(ui8 idx) -> std::string
  {
    return "\x1b[38;5;" + std::to_string((int)idx) + "m";
  }

  static inline auto bg256(ui8 idx) -> std::string
  {
    return "\x1b[48;5;" + std::to_string((int)idx) + "m";
  }

  // ----- Basic 16 colors -----
  static inline auto fg16(Basic16 c) -> std::string
  {
    const int base = (int)c;
    if (base < 8)
      return "\x1b[" + std::to_string(30 + base) + "m";
    return "\x1b[" + std::to_string(90 + (base - 8)) + "m";
  }

  static inline auto bg16(Basic16 c) -> std::string
  {
    const int base = (int)c;
    if (base < 8)
      return "\x1b[" + std::to_string(40 + base) + "m";
    return "\x1b[" + std::to_string(100 + (base - 8)) + "m";
  }

  static inline auto MoveCursor(int row, int col) -> std::string
  {
    // 1-based rows/cols
    return "\x1b[" + std::to_string(row) + ";" + std::to_string(col) + "H";
  }

  static inline auto HideCursor() -> std::string { return "\x1b[?25l"; }

  static inline auto ShowCursor() -> std::string { return "\x1b[?25h"; }
};

// ============================================================
// Conversions: RGBA -> ANSI 256 (simple mapping)
// ============================================================
//
// This is a classic approximation:
// - If r=g=b -> grayscale ramp
// - Otherwise map to 6x6x6 cube
//
constexpr auto clampU8(int x) -> ui8 { return (ui8)(x < 0 ? 0 : (x > 255 ? 255 : x)); }

constexpr auto nearestCubeIndex(ui8 v) -> ui8
{
  // 0..255 -> 0..5
  // (value * 5 + 127) / 255 for rounding
  return (ui8)((v * 5 + 127) / 255);
}

constexpr auto cubeLevel(ui8 idx) -> ui8
{
  // 0, 95, 135, 175, 215, 255
  // official xterm table levels
  return (idx == 0) ? 0 : (ui8)(55 + 40 * idx);
}

constexpr auto toAnsi256(utils::colors::RGBA c) -> ui8
{
  // grayscale special-casing
  if (c.r == c.g && c.g == c.b)
  {
    if (c.r < 8)
      return 16;
    if (c.r > 248)
      return 231;

    // 24 grayscale levels in xterm palette
    const int gray = (c.r - 8) / 10; // 0..23
    return (ui8)(232 + gray);
  }

  const ui8 ri = nearestCubeIndex(c.r);
  const ui8 gi = nearestCubeIndex(c.g);
  const ui8 bi = nearestCubeIndex(c.b);

  return (ui8)(16 + 36 * ri + 6 * gi + bi);
}

} // namespace utils::colors
