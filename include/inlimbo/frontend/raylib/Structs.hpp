#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include "utils/string/SmallString.hpp"
#include <raylib.h>
#include <string_view>

namespace frontend::raylib
{

inline auto keyFromString(std::string_view sv) -> int
{
  if (sv.empty())
    return 0;

  auto s = utils::string::transform::tolower_ascii(sv.data());

  // Common named keys
  if (s == "space")
    return KEY_SPACE;
  if (s == "enter")
    return KEY_ENTER;
  if (s == "esc" || s == "escape")
    return KEY_ESCAPE;
  if (s == "tab")
    return KEY_TAB;
  if (s == "backspace")
    return KEY_BACKSPACE;

  if (s == "up")
    return KEY_UP;
  if (s == "down")
    return KEY_DOWN;
  if (s == "left")
    return KEY_LEFT;
  if (s == "right")
    return KEY_RIGHT;

  if (s.size() == 1)
  {
    char c = s.c_str()[0];

    if (c >= 'a' && c <= 'z')
      return KEY_A + (c - 'a');

    if (c >= '0' && c <= '9')
      return KEY_ZERO + (c - '0');

    switch (c)
    {
      case ' ':
        return KEY_SPACE;
      case '=':
        return KEY_EQUAL;
      case '-':
        return KEY_MINUS;
      case '/':
        return KEY_SLASH;
      case '.':
        return KEY_PERIOD;
      case ',':
        return KEY_COMMA;
      case ';':
        return KEY_SEMICOLON;
      case '\'':
        return KEY_APOSTROPHE;
      case '[':
        return KEY_LEFT_BRACKET;
      case ']':
        return KEY_RIGHT_BRACKET;
      case '\\':
        return KEY_BACKSLASH;
      case '`':
        return KEY_GRAVE;
      default:
        break;
    }
  }

  return 0;
}

struct Keybinds
{
  int playPause{KEY_P};
  int next{KEY_N};
  int prev{KEY_B};
  int random{KEY_X};
  int songInfo{KEY_I};
  int restart{KEY_R};
  int seekBack{KEY_J};
  int seekFwd{KEY_K};
  int volUp{KEY_EQUAL};
  int volDown{KEY_MINUS};
  int quit{KEY_Q};

  utils::string::SmallString playPauseName{"p"};
  utils::string::SmallString nextName{"n"};
  utils::string::SmallString prevName{"b"};
  utils::string::SmallString songInfoName{"i"};
  utils::string::SmallString randomName{"x"};
  utils::string::SmallString restartName{"r"};
  utils::string::SmallString seekBackName{"j"};
  utils::string::SmallString seekFwdName{"k"};
  utils::string::SmallString volUpName{"="};
  utils::string::SmallString volDownName{"-"};
  utils::string::SmallString quitName{"q"};

  static auto load(std::string_view frontend) -> Keybinds
  {
    Keybinds out{};

    auto& kb = config::keybinds::Registry::theme(frontend);

    auto readKey = [&](const char* keyName, std::string_view fallbackStr, int fallbackKey,
                       int& outKey, utils::string::SmallString& outName) -> void
    {
      auto nameStr = kb.getKeyName(keyName, fallbackStr);

      outName = nameStr;

      int k  = keyFromString(nameStr);
      outKey = (k != 0) ? k : fallbackKey;
    };

    readKey("play_pause", "p", out.playPause, out.playPause, out.playPauseName);
    readKey("next", "n", out.next, out.next, out.nextName);
    readKey("prev", "b", out.prev, out.prev, out.prevName);
    readKey("song_info", "/", out.songInfo, out.songInfo, out.songInfoName);
    readKey("random", "x", out.random, out.random, out.randomName);
    readKey("restart", "r", out.restart, out.restart, out.restartName);
    readKey("seek_back", "j", out.seekBack, out.seekBack, out.seekBackName);
    readKey("seek_fwd", "k", out.seekFwd, out.seekFwd, out.seekFwdName);
    readKey("vol_up", "=", out.volUp, out.volUp, out.volUpName);
    readKey("vol_down", "-", out.volDown, out.volDown, out.volDownName);
    readKey("quit", "q", out.quit, out.quit, out.quitName);

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
