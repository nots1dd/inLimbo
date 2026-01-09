#include "frontend/raylib/view/Header.hpp"
#include "frontend/raylib/Constants.hpp"

namespace frontend::raylib::view
{

auto Header::infoButton() const -> Rectangle
{
  return {WIN_W - 40, (HEADER_H - 24) * 0.5f, 24, 24};
}

void Header::draw(const ui::Fonts& fonts, state::UI& ui)
{
  DrawRectangle(0, 0, WIN_W, HEADER_H, BG_PANEL);
  DrawLine(0, HEADER_H, WIN_W, HEADER_H, {60, 60, 60, 255});

  DrawTextEx(fonts.bold, "InLimbo Player", {16, 14}, 22, 1, ACCENT);

  const char* title = ui.screen == state::UI::Screen::Library ? "Library" : "Now Playing";

  int tw = MeasureTextEx(fonts.regular, title, 18, 1).x;
  DrawTextEx(fonts.regular, title, {(float)(WIN_W - tw - 50), 16}, 18, 1, TEXT_DIM);

  Rectangle btn   = infoButton();
  bool      hover = CheckCollisionPointRec(GetMousePosition(), btn);

  DrawRectangleRounded(btn, 0.3f, 6, hover ? ACCENT : BG_PANEL);
  DrawTextEx(fonts.regular, "(i)", {btn.x + 4, btn.y + 2}, 18, 1, hover ? WHITE : TEXT_DIM);

  if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    ui.showMetaInfo = !ui.showMetaInfo;
}

} // namespace frontend::raylib::view
