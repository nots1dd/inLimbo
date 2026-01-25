#pragma once

#include "InLimbo-Types.hpp"
#include "utils/map/AllocOptimization.hpp"
#include <string>
#include <string_view>
#include <unordered_map>

namespace config::colors
{

using Member = utils::string::SmallString;

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

enum class Layer : ui8
{
  Foreground,
  Background
};

enum class Mode : ui8
{
  TrueColor24,
  Color256,
  Basic16
};

enum class Policy : ui8
{
  Adaptive, // don't hard force RGB; respect terminal theme
  Forced    // always emit absolute colors
};

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
  static inline auto fgTrue(RGBA c) -> std::string
  {
    return "\x1b[38;2;" + std::to_string((int)c.r) + ";" + std::to_string((int)c.g) + ";" +
           std::to_string((int)c.b) + "m";
  }

  static inline auto bgTrue(RGBA c) -> std::string
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

constexpr auto toAnsi256(RGBA c) -> ui8
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

struct ColorValue
{
  RGBA rgba{};
  ui8  ansi256{0};

  constexpr ColorValue() = default;
  constexpr ColorValue(RGBA c) : rgba(c) {}

  static auto fromRGBA(RGBA c) -> ColorValue
  {
    ColorValue v;
    v.rgba    = c;
    v.ansi256 = toAnsi256(c);
    return v;
  }
};

class Theme
{
public:
  Theme() = default;
  explicit Theme(std::string name) : m_name(std::move(name)) {}

  auto name() const noexcept -> const std::string& { return m_name; }

  // Add/update a color
  auto set(std::string key, RGBA value) -> Theme&
  {
    m_colors[std::move(key)] = ColorValue::fromRGBA(value);
    return *this;
  }

  auto has(std::string_view key) const -> bool { return m_colors.find(key) != m_colors.end(); }

  auto get(std::string_view key, RGBA fallback = RGBA{}) const -> RGBA
  {
    auto it = m_colors.find(key);
    if (it == m_colors.end())
      return fallback;
    return it->second.rgba;
  }

  auto get256(std::string_view key, ui8 fallback = 15) const -> ui8
  {
    auto it = m_colors.find(key);
    if (it == m_colors.end())
      return fallback;
    return it->second.ansi256;
  }

  // Generate ANSI escape for theme color
  auto ansi(std::string_view key, Layer layer = Layer::Foreground,
            Mode mode = Mode::TrueColor24) const -> std::string
  {
    auto it = m_colors.find(key);
    if (it == m_colors.end())
      return std::string(Ansi::Reset);

    const auto& c = it->second;

    if (mode == Mode::TrueColor24)
      return (layer == Layer::Foreground) ? Ansi::fgTrue(c.rgba) : Ansi::bgTrue(c.rgba);

    if (mode == Mode::Color256)
      return (layer == Layer::Foreground) ? Ansi::fg256(c.ansi256) : Ansi::bg256(c.ansi256);

    return std::string(Ansi::Reset);
  }

private:
  std::string m_name{"default"};
  std::unordered_map<std::string, ColorValue, utils::map::TransparentHash,
                     utils::map::TransparentEq>
    m_colors;
};

class Registry
{
public:
  static auto setActive(std::string name) -> void { s_active = std::move(name); }

  static auto activeName() -> const std::string& { return s_active; }

  static auto put(Theme theme) -> void { s_themes[theme.name()] = std::move(theme); }

  static auto theme(std::string_view name) -> Theme& { return s_themes[std::string(name)]; }

  static auto active() -> Theme& { return s_themes[s_active]; }

  static auto setForced(bool enabled) -> void { s_forceColors = enabled; }
  static auto forced() noexcept -> bool { return s_forceColors; }

private:
  static inline std::unordered_map<std::string, Theme> s_themes{};
  static inline std::string                            s_active{"default"};
  static inline bool                                   s_forceColors{true};
};

} // namespace config::colors
