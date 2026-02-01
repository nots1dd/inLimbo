#pragma once

#include "Logger.hpp"
#include "config/Bind.hpp"
#include "toml/Parser.hpp"
#include "utils/Colors.hpp"
#include <functional>
#include <string>
#include <unordered_map>

namespace config::colors
{

using Member = utils::string::SmallString;

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

// will check if apply is required or not,
// this is just a dummy bind so we are leaving
// it empty.
//
// in actuality, we are using ANSI scheme to bind
inline auto bind(std::string_view key, utils::colors::RGBA& out)
  -> config::colors::Binding<utils::colors::RGBA>
{
  return {.key = key, .target = &out, .apply = {}};
}

inline auto bindAnsi(std::string_view key, std::string& out, colors::Layer layer, colors::Mode mode)
  -> colors::Binding<std::string>
{
  return {.key    = key,
          .target = &out,
          .apply  = [=](const utils::colors::RGBA& c, std::string& dst) -> void
          {
            using A = config::colors::Ansi;
            if (mode == Mode::TrueColor24)
              dst = (layer == Layer::Foreground) ? A::fgTrue(c) : A::bgTrue(c);
            else if (mode == Mode::Color256)
              dst = (layer == Layer::Foreground) ? A::fg256(toAnsi256(c)) : A::bg256(toAnsi256(c));
            else
              dst = A::Reset;
          }};
}

class ConfigLoader
{
public:
  explicit ConfigLoader(std::string_view frontend) : m_frontend(std::move(frontend)) {}

  template <typename... Bindings> auto load(Bindings&&... bindings) const -> void
  {
    if (!tomlparser::Config::isLoaded())
      throw std::runtime_error("Colors::ConfigLoader: toml not loaded");

    const auto& root = tomlparser::Config::table("colors");
    const auto* node = root.get(m_frontend);
    if (!node)
      return;

    const auto* tbl = node->as_table();
    if (!tbl)
      return;

    using Handler = std::function<void(const toml::node&)>;
    std::unordered_map<std::string_view, Handler> handlers;

    (registerBinding(handlers, std::forward<Bindings>(bindings)), ...);

    for (const auto& [k, v] : *tbl)
    {
      auto it = handlers.find(k.str());
      if (it != handlers.end())
        it->second(v);
      else
        LOG_WARN("Unknown color key: colors.{}.{}", m_frontend, k.str());
    }
  }

private:
  std::string m_frontend;

  template <typename T>
  static auto registerBinding(
    std::unordered_map<std::string_view, std::function<void(const toml::node&)>>& handlers,
    const Binding<T>&                                                             b) -> void
  {
    handlers[b.key] = [ptr = b.target, apply = b.apply, key = b.key](const toml::node& n) -> auto
    {
      auto str = n.value<std::string>();
      if (!str)
      {
        LOG_WARN("Color '{}' must be string", key);
        return;
      }

      auto rgba = utils::colors::parseRGBA(*str);
      if (!rgba)
      {
        LOG_WARN("Invalid color '{}'", key);
        return;
      }

      if constexpr (std::is_same_v<T, utils::colors::RGBA>)
      {
        *ptr = *rgba;
      }
      else
      {
        // non-RGBA targets must provide apply()
        if (!apply)
        {
          LOG_ERROR("Color binding '{}' requires custom apply() for target type", key);
          return;
        }
        apply(*rgba, *ptr);
      }
    };
  }
};

} // namespace config::colors
