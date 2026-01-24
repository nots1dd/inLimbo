#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include <raylib.h>
#include <string_view>

namespace frontend::raylib
{

using RaylibKey = int;

struct Keybinds
{
  RaylibKey playPause;
  RaylibKey next;
  RaylibKey prev;
  RaylibKey random;
  RaylibKey songInfo;
  RaylibKey restart;
  RaylibKey seekBack;
  RaylibKey seekFwd;
  RaylibKey volUp;
  RaylibKey volDown;
  RaylibKey quit;

  KeyName playPauseName;
  KeyName nextName;
  KeyName prevName;
  KeyName songInfoName;
  KeyName randomName;
  KeyName restartName;
  KeyName seekBackName;
  KeyName seekFwdName;
  KeyName volUpName;
  KeyName volDownName;
  KeyName quitName;

  static auto load(std::string_view frontend) -> Keybinds
  {
    Keybinds out{};

    auto& kb = config::keybinds::Registry::theme(frontend);

    out.playPause = kb.getAs<RaylibKey>("play_pause", KEY_P);
    out.next      = kb.getAs<RaylibKey>("next", KEY_N);
    out.prev      = kb.getAs<RaylibKey>("prev", KEY_B);
    out.songInfo  = kb.getAs<RaylibKey>("song_info", KEY_I);
    out.random    = kb.getAs<RaylibKey>("random", KEY_X);
    out.restart   = kb.getAs<RaylibKey>("restart", KEY_R);
    out.seekBack  = kb.getAs<RaylibKey>("seek_back", KEY_J);
    out.seekFwd   = kb.getAs<RaylibKey>("seek_fwd", KEY_K);
    out.volUp     = kb.getAs<RaylibKey>("vol_up", KEY_EQUAL);
    out.volDown   = kb.getAs<RaylibKey>("vol_down", KEY_MINUS);
    out.quit      = kb.getAs<RaylibKey>("quit", KEY_Q);

    out.playPauseName = kb.getKeyName("play_pause", "p");
    out.nextName      = kb.getKeyName("next", "n");
    out.prevName      = kb.getKeyName("prev", "b");
    out.songInfoName  = kb.getKeyName("song_info", "i");
    out.randomName    = kb.getKeyName("random", "x");
    out.restartName   = kb.getKeyName("restart", "r");
    out.seekBackName  = kb.getKeyName("seek_back", "j");
    out.seekFwdName   = kb.getKeyName("seek_fwd", "k");
    out.volUpName     = kb.getKeyName("vol_up", "=");
    out.volDownName   = kb.getKeyName("vol_down", "-");
    out.quitName      = kb.getKeyName("quit", "q");

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
