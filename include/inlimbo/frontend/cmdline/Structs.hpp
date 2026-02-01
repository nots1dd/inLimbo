#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include "frontend/KV.hpp"

namespace frontend::cmdline
{

using CmdlineKey = char;

using CmdlineKV = KeyValueBase<CmdlineKey, KeyName>;

inline auto bind(std::string_view key, CmdlineKV& kv) -> config::keybinds::Binding<CmdlineKV>
{
  return config::keybinds::Binding<CmdlineKV>{.key = key, .target = &kv};
}

struct Keybinds
{
  CmdlineKV playPause;
  CmdlineKV nextTrack;
  CmdlineKV prevTrack;
  CmdlineKV randomTrack;
  CmdlineKV searchTitle;
  CmdlineKV searchArtist;
  CmdlineKV restartTrack;
  CmdlineKV seekBack;
  CmdlineKV seekFwd;
  CmdlineKV volUp;
  CmdlineKV volDown;
  CmdlineKV quit;

  static auto load(std::string_view frontend) -> Keybinds
  {
    // defaults
    Keybinds out{
      .playPause    = {'p', "p"},
      .nextTrack    = {'n', "n"},
      .prevTrack    = {'b', "b"},
      .randomTrack  = {'x', "x"},
      .searchTitle  = {'/', "/"},
      .searchArtist = {'a', "a"},
      .restartTrack = {'r', "r"},
      .seekBack     = {'j', "j"},
      .seekFwd      = {'k', "k"},
      .volUp        = {'=', "="},
      .volDown      = {'-', "-"},
      .quit         = {'q', "q"},
    };

    // -----------------------------
    // Binding-based loader
    // -----------------------------
    config::keybinds::ConfigLoader loader(frontend);

    loader.load(bind("play_pause", out.playPause), bind("next_track", out.nextTrack),
                bind("prev_track", out.prevTrack), bind("random_track", out.randomTrack),
                bind("search_title", out.searchTitle), bind("search_artist", out.searchArtist),
                bind("restart_track", out.restartTrack), bind("seek_back", out.seekBack),
                bind("seek_fwd", out.seekFwd), bind("vol_up", out.volUp),
                bind("vol_down", out.volDown), bind("quit", out.quit));

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

struct MiscConfig
{
  int seekDuration;
};

struct CmdlineConfig
{
  Keybinds   kb;
  UiColors   colors;
  MiscConfig misc;
};

} // namespace frontend::cmdline
