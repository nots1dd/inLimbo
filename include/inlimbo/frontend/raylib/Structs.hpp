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
    Keybinds out{};

    auto& kb = config::keybinds::Registry::theme(frontend);

    // raylib maps keys in a different enum so simply giving the handler ascii
    // will NOT work! (as it uses GetKeyPressed() which is a raylib func)
    //
    // So we need a constexpr func to map ascii -> raylib key codes
    auto l_loadKeyAndName = [&](RaylibKV& k, std::string_view id, RaylibKey fallback,
                                std::string_view nameFallback) -> void
    {
      k.key  = asciiToRaylib(kb.getAs<RaylibKey>(id, fallback));
      k.name = kb.getKeyName(id, nameFallback);
    };

    l_loadKeyAndName(out.playPause, "play_pause", 'p', "p");
    l_loadKeyAndName(out.nextTrack, "next_track", 'n', "n");
    l_loadKeyAndName(out.prevTrack, "prev_track", 'b', "b");
    l_loadKeyAndName(out.songInfo, "song_info", 'i', "i");
    l_loadKeyAndName(out.randomTrack, "random_track", 'x', "x");
    l_loadKeyAndName(out.restart, "restart", 'r', "r");
    l_loadKeyAndName(out.seekBack, "seek_back", 'j', "j");
    l_loadKeyAndName(out.seekFwd, "seek_fwd", 'k', "k");
    l_loadKeyAndName(out.volUp, "vol_up", '=', "=");
    l_loadKeyAndName(out.volDown, "vol_down", '-', "-");
    l_loadKeyAndName(out.quit, "quit", 'q', "q");

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
    UiColors out{};

    auto& t = config::colors::Registry::theme(frontend);

    out.fg = t.ansi("fg", config::colors::Layer::Foreground, config::colors::Mode::TrueColor24);
    out.bg = t.ansi("bg", config::colors::Layer::Background, config::colors::Mode::TrueColor24);
    out.accent =
      t.ansi("accent", config::colors::Layer::Foreground, config::colors::Mode::TrueColor24);
    out.warning =
      t.ansi("warning", config::colors::Layer::Foreground, config::colors::Mode::TrueColor24);
    out.error =
      t.ansi("error", config::colors::Layer::Foreground, config::colors::Mode::TrueColor24);
    out.success =
      t.ansi("success", config::colors::Layer::Foreground, config::colors::Mode::TrueColor24);

    return out;
  }
};

struct RaylibConfig
{
  Keybinds kb;
  UiColors colors;
};

} // namespace frontend::raylib
