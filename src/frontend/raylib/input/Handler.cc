#include "frontend/raylib/input/Handler.hpp"
#include <algorithm>
#include <raylib.h>

namespace frontend::raylib::input
{

void Handler::handle(audio::Service& audio, state::UI& ui, state::Library& lib,
                     mpris::Service* mpris)
{
  auto info = audio.getCurrentTrackInfo();

  if (info)
  {
    if (info->lengthSec > 0 && info->positionSec > info->lengthSec)
    {
      audio.nextTrack();
      if (mpris)
        mpris->updateMetadata();
    }
  }

  bool trackChanged = false;

  if (IsKeyPressed(KEY_Q))
    ui.running = false;

  if (IsKeyPressed(KEY_I))
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

  if (IsKeyPressed(KEY_P))
    audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();

  if (IsKeyPressed(KEY_N))
  {
    audio.nextTrack();
    trackChanged = true;
  }

  if (IsKeyPressed(KEY_B))
  {
    audio.previousTrack();
    trackChanged = true;
  }

  if (IsKeyPressed(KEY_EQUAL))
    audio.setVolume(std::min(1.5f, audio.getVolume() + 0.05f));

  if (IsKeyPressed(KEY_MINUS))
    audio.setVolume(std::max(0.0f, audio.getVolume() - 0.05f));

  if (trackChanged && mpris)
    mpris->updateMetadata();

  mpris->notify();
}

} // namespace frontend::raylib::input
