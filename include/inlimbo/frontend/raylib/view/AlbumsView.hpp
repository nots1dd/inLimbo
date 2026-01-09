#pragma once

#include "audio/Service.hpp"
#include "frontend/raylib/state/Library.hpp"
#include "frontend/raylib/ui/Fonts.hpp"
#include "mpris/Service.hpp"
#include "thread/Map.hpp"

namespace frontend::raylib::view
{

class AlbumsView
{
public:
  void draw(const ui::Fonts& fonts, state::Library& lib, audio::Service& audio,
            threads::SafeMap<SongMap>& songs, mpris::Service& mpris);

};

} // namespace frontend::raylib::view
