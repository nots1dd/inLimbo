#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include "utils/string/SmallString.hpp"

namespace frontend::cmdline
{

struct Keybinds
{
  char playPause;
  char next;
  char prev;
  char random;
  char searchSong;
  char restart;
  char seekBack;
  char seekFwd;
  char volUp;
  char volDown;
  char quit;

  utils::string::SmallString playPauseName;
  utils::string::SmallString nextName;
  utils::string::SmallString prevName;
  utils::string::SmallString searchSongName;
  utils::string::SmallString randomName;
  utils::string::SmallString restartName;
  utils::string::SmallString seekBackName;
  utils::string::SmallString seekFwdName;
  utils::string::SmallString volUpName;
  utils::string::SmallString volDownName;
  utils::string::SmallString quitName;

  static auto load(std::string_view frontend) -> Keybinds
  {
    Keybinds out{};

    auto& kb = config::keybinds::Registry::theme(frontend);

    out.playPause  = kb.getChar("play_pause", 'p');
    out.next       = kb.getChar("next", 'n');
    out.prev       = kb.getChar("prev", 'b');
    out.searchSong = kb.getChar("search_song", '/');
    out.random     = kb.getChar("random", 'x');
    out.restart    = kb.getChar("restart", 'r');
    out.seekBack   = kb.getChar("seek_back", 'j');
    out.seekFwd    = kb.getChar("seek_fwd", 'k');
    out.volUp      = kb.getChar("vol_up", '=');
    out.volDown    = kb.getChar("vol_down", '-');
    out.quit       = kb.getChar("quit", 'q');

    out.playPauseName  = kb.getKeyName("play_pause", "p");
    out.nextName       = kb.getKeyName("next", "n");
    out.prevName       = kb.getKeyName("prev", "b");
    out.searchSongName = kb.getKeyName("search_song", "/");
    out.randomName     = kb.getKeyName("random", "x");
    out.restartName    = kb.getKeyName("restart", "r");
    out.seekBackName   = kb.getKeyName("seek_back", "j");
    out.seekFwdName    = kb.getKeyName("seek_fwd", "k");
    out.volUpName      = kb.getKeyName("vol_up", "=");
    out.volDownName    = kb.getKeyName("vol_down", "-");
    out.quitName       = kb.getKeyName("quit", "q");

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

struct CmdlineConfig
{
  Keybinds kb;
  UiColors colors;
};

} // namespace frontend::cmdline
