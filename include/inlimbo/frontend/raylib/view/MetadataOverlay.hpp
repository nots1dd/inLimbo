#pragma once

#include "InLimbo-Types.hpp"
#include "frontend/raylib/ui/Fonts.hpp"

namespace frontend::raylib::view
{

class MetadataOverlay
{
public:
  void draw(const ui::Fonts& fonts, const Metadata& meta);
};

} // namespace frontend::raylib::view
