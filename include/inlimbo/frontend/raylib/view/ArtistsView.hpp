#pragma once

#include "audio/Service.hpp"
#include "frontend/raylib/state/Library.hpp"
#include "frontend/raylib/ui/Fonts.hpp"

namespace frontend::raylib::view
{

class ArtistsView
{
public:
  void draw(const ui::Fonts& fonts, state::Library& lib, audio::Service& audio);
};

} // namespace frontend::raylib::view
