#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"
#include "frontend/KV.hpp"

namespace frontend::cmdline
{

using CmdlineKey = char;

using CmdlineKV = KeyValueBase<CmdlineKey, KeyName>;

struct Keybinds
{
  CmdlineKV playPause;
  CmdlineKV nextTrack;
  CmdlineKV prevTrack;
  CmdlineKV randomTrack;
  CmdlineKV searchTitle;
  CmdlineKV searchArtist;
  CmdlineKV toggleAVBars;
  CmdlineKV restartTrack;
  CmdlineKV seekBack;
  CmdlineKV seekFwd;
  CmdlineKV volUp;
  CmdlineKV volDown;
  CmdlineKV quit;

  static auto load(std::string_view frontend) -> Keybinds
  {
    Keybinds out{};
    auto&    kb = config::keybinds::Registry::theme(frontend);

    auto l_loadKeyAndName = [&](CmdlineKV& k, std::string_view id, CmdlineKey fallback,
                                std::string_view nameFallback) -> void
    {
      k.key  = kb.getAs<CmdlineKey>(id, fallback);
      k.name = kb.getKeyName(id, nameFallback);
    };

    l_loadKeyAndName(out.playPause, "play_pause", 'p', "p");
    l_loadKeyAndName(out.nextTrack, "next_track", 'n', "n");
    l_loadKeyAndName(out.prevTrack, "prev_track", 'b', "b");
    l_loadKeyAndName(out.searchTitle, "search_title", '/', "/");
    l_loadKeyAndName(out.searchArtist, "search_artist", 'a', "a");
    l_loadKeyAndName(out.toggleAVBars, "toggle_av_bars", 'v', "v");
    l_loadKeyAndName(out.randomTrack, "random_track", 'x', "x");
    l_loadKeyAndName(out.restartTrack, "restart_track", 'r', "r");
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

struct CmdlineConfig
{
  Keybinds kb;
  UiColors colors;
};

} // namespace frontend::cmdline
