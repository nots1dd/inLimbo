#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include "frontend/KV.hpp"
#include <raylib.h>
#include <string_view>

namespace frontend::raylib
{

using RaylibKey = int;

using RaylibKV = KeyValueBase<RaylibKey, KeyName>;

[[nodiscard]]
constexpr auto asciiToRaylib(int ascii, RaylibKey fallback = KEY_NULL) noexcept -> RaylibKey
{
  // a-z
  if (ascii >= 'a' && ascii <= 'z')
    return KEY_A + (ascii - 'a');

  // A-Z
  if (ascii >= 'A' && ascii <= 'Z')
    return KEY_A + (ascii - 'A');

  // 0-9
  if (ascii >= '0' && ascii <= '9')
    return KEY_ZERO + (ascii - '0');

  // Common symbols (minimal table)
  //
  // Note that raylib seems to be weird with
  // certain shifted numeric keys like [!@..*]
  //
  // Example: `+` (Plus on keypad works with KEY_KP_ADD)
  // however, the simple plus key we get with SHIFT+EQUAL
  // is not present in their enum.
  //
  // Any keybind issue is considered a sole frontend issue,
  // and solely depends on the framework being used along with
  // how it was implemented.
  switch (ascii)
  {
    case ' ':
      return KEY_SPACE;
    case '-':
      return KEY_MINUS;
    case '=':
      return KEY_EQUAL;
    case '[':
      return KEY_LEFT_BRACKET;
    case ']':
      return KEY_RIGHT_BRACKET;
    case ';':
      return KEY_SEMICOLON;
    case '\'':
      return KEY_APOSTROPHE;
    case ',':
      return KEY_COMMA;
    case '.':
      return KEY_PERIOD;
    case '/':
      return KEY_SLASH;
    case '\\':
      return KEY_BACKSLASH;
    case '`':
      return KEY_GRAVE;
    default:
      return fallback;
  }
}

inline auto bind(std::string_view key, RaylibKV& kv) -> config::keybinds::Binding<RaylibKV>
{
  return {.key    = key,
          .target = &kv,
          .apply  = [](char ascii, RaylibKV& out) -> void
          {
            out.key  = asciiToRaylib(ascii);
            out.name = config::keybinds::parseKeyName(ascii);
          }};
}

struct Keybinds
{
  RaylibKV playPause;
  RaylibKV nextTrack;
  RaylibKV prevTrack;
  RaylibKV randomTrack;
  RaylibKV songInfo;
  RaylibKV restart;
  RaylibKV seekBack;
  RaylibKV seekFwd;
  RaylibKV volUp;
  RaylibKV volDown;
  RaylibKV quit;

  static auto load(std::string_view frontend) -> Keybinds
  {
    // defaults
    Keybinds out{
      .playPause   = {KEY_P, "p"},
      .nextTrack   = {KEY_N, "n"},
      .prevTrack   = {KEY_B, "b"},
      .randomTrack = {KEY_X, "x"},
      .songInfo    = {KEY_I, "i"},
      .restart     = {KEY_R, "r"},
      .seekBack    = {KEY_J, "j"},
      .seekFwd     = {KEY_K, "k"},
      .volUp       = {KEY_EQUAL, "="},
      .volDown     = {KEY_MINUS, "-"},
      .quit        = {KEY_Q, "q"},
    };

    config::keybinds::ConfigLoader loader(frontend);

    loader.load(bind("play_pause", out.playPause), bind("next_track", out.nextTrack),
                bind("prev_track", out.prevTrack), bind("random_track", out.randomTrack),
                bind("song_info", out.songInfo), bind("restart", out.restart),
                bind("seek_back", out.seekBack), bind("seek_fwd", out.seekFwd),
                bind("vol_up", out.volUp), bind("vol_down", out.volDown), bind("quit", out.quit));

    return out;
  }
};

struct UiColors
{
  std::string fg;
  std::string bg;

  std::string accent;
  std::string warning;
  std::string error;
  std::string success;

  static auto load(std::string_view frontend) -> UiColors
  {
    using namespace config::colors;

    UiColors out{};

    ConfigLoader loader(frontend);

    loader.load(bindAnsi("fg", out.fg, Layer::Foreground, Mode::TrueColor24),
                bindAnsi("bg", out.bg, Layer::Background, Mode::TrueColor24),
                bindAnsi("accent", out.accent, Layer::Foreground, Mode::TrueColor24),
                bindAnsi("warning", out.warning, Layer::Foreground, Mode::TrueColor24),
                bindAnsi("error", out.error, Layer::Foreground, Mode::TrueColor24),
                bindAnsi("success", out.success, Layer::Foreground, Mode::TrueColor24));

    return out;
  }
};

struct RaylibConfig
{
  Keybinds kb;
  UiColors colors;
};

} // namespace frontend::raylib
