#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include "utils/Snapshot.hpp"
#include "utils/string/SmallString.hpp"

namespace frontend::cmdline
{
struct Keybinds
{
  char playPause{'p'};
  char next{'n'};
  char prev{'P'};
  char restart{'r'};
  char seekBack{'b'};
  char seekFwd{'f'};
  char volUp{'='};
  char volDown{'-'};
  char quit{'q'};

  utils::string::SmallString playPauseName{"p"};
  utils::string::SmallString nextName{"n"};
  utils::string::SmallString prevName{"P"};
  utils::string::SmallString restartName{"r"};
  utils::string::SmallString seekBackName{"b"};
  utils::string::SmallString seekFwdName{"f"};
  utils::string::SmallString volUpName{"="};
  utils::string::SmallString volDownName{"-"};
  utils::string::SmallString quitName{"q"};

  static auto load(std::string_view frontend) -> Keybinds
  {
    Keybinds out{};

    auto& kb = config::keybinds::Registry::theme(frontend);

    out.playPause = kb.getChar("play_pause", out.playPause);
    out.next      = kb.getChar("next", out.next);
    out.prev      = kb.getChar("prev", out.prev);
    out.restart   = kb.getChar("restart", out.restart);
    out.seekBack  = kb.getChar("seek_back", out.seekBack);
    out.seekFwd   = kb.getChar("seek_fwd", out.seekFwd);
    out.volUp     = kb.getChar("vol_up", out.volUp);
    out.volDown   = kb.getChar("vol_down", out.volDown);
    out.quit      = kb.getChar("quit", out.quit);

    out.playPauseName = kb.getKeyName("play_pause", "p");
    out.nextName      = kb.getKeyName("next", "n");
    out.prevName      = kb.getKeyName("prev", "P");
    out.restartName   = kb.getKeyName("restart", "r");
    out.seekBackName  = kb.getKeyName("seek_back", "b");
    out.seekFwdName   = kb.getKeyName("seek_fwd", "f");
    out.volUpName     = kb.getKeyName("vol_up", "=");
    out.volDownName   = kb.getKeyName("vol_down", "-");
    out.quitName      = kb.getKeyName("quit", "q");

    return out;
  }
};

struct UiColors
{
  // Foreground/background "themes"
  std::string fg;
  std::string bg;

  // Common semantic colors
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
