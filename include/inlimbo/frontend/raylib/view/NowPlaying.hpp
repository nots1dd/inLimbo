#pragma once

#include "audio/Service.hpp"
#include "frontend/raylib/media/AlbumArtCache.hpp"
#include "frontend/raylib/ui/Fonts.hpp"
#include "mpris/Service.hpp"

namespace frontend::raylib::view
{

class NowPlaying
{
public:
  void draw(const ui::Fonts& fonts, audio::Service& audio, mpris::Service& mpris);

private:
  media::AlbumArtCache m_art;

  void drawArt(const Metadata&);
  void drawProgress(const audio::service::TrackInfo&, audio::Service&, const ui::Fonts&);
  void drawControls(audio::Service&, const ui::Fonts&, mpris::Service& mpris);
};

} // namespace frontend::raylib::view
