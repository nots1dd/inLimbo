#pragma once

#include "frontend/raylib/state/UI.hpp"
#include "frontend/raylib/ui/Fonts.hpp"

namespace frontend::raylib::view
{

class Header
{
public:
  void draw(const ui::Fonts& fonts, state::UI& ui);

private:
  [[nodiscard]] auto infoButton() const -> Rectangle;
};

} // namespace frontend::raylib::view
