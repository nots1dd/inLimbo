#pragma once

#include "audio/Service.hpp"
#include "frontend/raylib/ui/Fonts.hpp"

namespace frontend::raylib::view
{

class StatusBar
{
public:
  void draw(const ui::Fonts& fonts, audio::Service& audio);
};

} // namespace frontend::raylib::view
