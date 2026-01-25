#pragma once

#include "config/Colors.hpp"
#include "config/Keybinds.hpp"

namespace frontend::cmdline
{

using CmdlineKey = char;

struct Keybinds
{
  CmdlineKey playPause;
  CmdlineKey nextTrack;
  CmdlineKey prevTrack;
  CmdlineKey randomTrack;
  CmdlineKey searchTitle;
  CmdlineKey searchArtist;
  CmdlineKey toggleAVBars;
  CmdlineKey restartTrack;
  CmdlineKey seekBack;
  CmdlineKey seekFwd;
  CmdlineKey volUp;
  CmdlineKey volDown;
  CmdlineKey quit;

  KeyName playPauseName;
  KeyName nextTrackName;
  KeyName prevTrackName;
  KeyName searchTitleName;
  KeyName searchArtistName;
  KeyName toggleAVBarsName;
  KeyName randomTrackName;
  KeyName restartTrackName;
  KeyName seekBackName;
  KeyName seekFwdName;
  KeyName volUpName;
  KeyName volDownName;
  KeyName quitName;

  static auto load(std::string_view frontend) -> Keybinds
  {
    Keybinds out{};

    auto& kb = config::keybinds::Registry::theme(frontend);

    out.playPause    = kb.getAs<CmdlineKey>("play_pause", 'p');
    out.nextTrack    = kb.getAs<CmdlineKey>("next_track", 'n');
    out.prevTrack    = kb.getAs<CmdlineKey>("prev_track", 'b');
    out.searchTitle  = kb.getAs<CmdlineKey>("search_title", '/');
    out.searchArtist = kb.getAs<CmdlineKey>("search_artist", 'a');
    out.toggleAVBars = kb.getAs<CmdlineKey>("toggle_av_bars", 'v');
    out.randomTrack  = kb.getAs<CmdlineKey>("random_track", 'x');
    out.restartTrack = kb.getAs<CmdlineKey>("restart_track", 'r');
    out.seekBack     = kb.getAs<CmdlineKey>("seek_back", 'j');
    out.seekFwd      = kb.getAs<CmdlineKey>("seek_fwd", 'k');
    out.volUp        = kb.getAs<CmdlineKey>("vol_up", '=');
    out.volDown      = kb.getAs<CmdlineKey>("vol_down", '-');
    out.quit         = kb.getAs<CmdlineKey>("quit", 'q');

    out.playPauseName    = kb.getKeyName("play_pause", "p");
    out.nextTrackName    = kb.getKeyName("next_track", "n");
    out.prevTrackName    = kb.getKeyName("prev_track", "b");
    out.searchTitleName  = kb.getKeyName("search_title", "/");
    out.searchArtistName = kb.getKeyName("search_artist", "a");
    out.toggleAVBarsName = kb.getKeyName("toggle_av_bars", "v");
    out.randomTrackName  = kb.getKeyName("random_track", "x");
    out.restartTrackName = kb.getKeyName("restart_track", "r");
    out.seekBackName     = kb.getKeyName("seek_back", "j");
    out.seekFwdName      = kb.getKeyName("seek_fwd", "k");
    out.volUpName        = kb.getKeyName("vol_up", "=");
    out.volDownName      = kb.getKeyName("vol_down", "-");
    out.quitName         = kb.getKeyName("quit", "q");

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
