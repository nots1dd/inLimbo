#include "frontend/raylib/input/Handler.hpp"
#include "frontend/raylib/Structs.hpp"
#include <algorithm>
#include <raylib.h>

namespace frontend::raylib::input
{

void Handler::handle(audio::Service& audio, state::UI& ui, const RaylibConfig& cfg,
                     mpris::Service* mpris)
{
  auto info = audio.getCurrentTrackInfo();

  if (info)
  {
    if (info->lengthSec > 0 && info->positionSec > info->lengthSec)
    {
      audio.nextTrackGapless();
      if (mpris)
        mpris->updateMetadata();
    }
  }

  bool trackChanged = false;

  if (IsKeyPressed(cfg.kb.quit()))
    ui.running = false;

  if (IsKeyPressed(cfg.kb.songInfo()))
  {
    if (ui.showMetaInfo)
    {
      ui.showMetaInfo = false;
    }
    else
    {
      ui.screen = (ui.screen == state::UI::Screen::Library) ? state::UI::Screen::NowPlaying
                                                            : state::UI::Screen::Library;
    }
  }

  if (IsKeyPressed(cfg.kb.playPause()))
    audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();

  if (IsKeyPressed(cfg.kb.nextTrack()))
  {
    audio.nextTrack();
    trackChanged = true;
  }

  if (IsKeyPressed(cfg.kb.randomTrack()))
  {
    audio.randomTrack();
    trackChanged = true;
  }

  if (IsKeyPressed(cfg.kb.prevTrack()))
  {
    audio.previousTrack();
    trackChanged = true;
  }

  if (IsKeyPressed(cfg.kb.restart()))
    audio.restartCurrent();

  if (IsKeyPressed(cfg.kb.volUp()))
    audio.setVolume(std::min(1.5f, audio.getVolume() + 0.05f));

  if (IsKeyPressed(cfg.kb.volDown()))
    audio.setVolume(std::max(0.0f, audio.getVolume() - 0.05f));

  if (IsKeyPressed(cfg.kb.seekFwd()))
  {
    audio.seekForward(3);
    mpris->notify();
  }
  if (IsKeyPressed(cfg.kb.seekBack()))
  {
    audio.seekBackward(3);
    mpris->notify();
  }

  if (trackChanged && mpris)
    mpris->updateMetadata();

  mpris->notify();
}

} // namespace frontend::raylib::input
