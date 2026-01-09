#include "frontend/raylib/view/ArtistsView.hpp"
#include "frontend/raylib/Constants.hpp"
#include "frontend/raylib/ui/TextUtils.hpp"
#include <raylib.h>

namespace frontend::raylib::view
{

static constexpr int   HEADER_H = 50;
static constexpr int   LEFT_W   = 260;
static constexpr float ITEM_H   = 24.f;
static constexpr float VIEW_H   = 700 - 50 - 48 - 48;

void ArtistsView::draw(const ui::Fonts& fonts, state::Library& lib, audio::Service&)
{
  Rectangle pane = {0, HEADER_H, LEFT_W, 700 - HEADER_H - 48};
  DrawRectangleRec(pane, BG_PANEL);

  DrawTextEx(fonts.bold, "Artists", {12, HEADER_H + 12}, 18, 1, TEXT_MAIN);
  DrawLine(8, HEADER_H + 38, LEFT_W - 8, HEADER_H + 38, {50, 50, 50, 255});

  Vector2 mouse = GetMousePosition();
  if (CheckCollisionPointRec(mouse, pane))
    lib.artistScrollY += GetMouseWheelMove() * 30;

  float contentH    = lib.artists.size() * ITEM_H;
  lib.artistScrollY = std::clamp(lib.artistScrollY, std::min(0.f, VIEW_H - contentH), 0.f);

  BeginScissorMode(0, HEADER_H + 48, LEFT_W, VIEW_H);

  float y = HEADER_H + 48 + lib.artistScrollY;

  for (size_t i = 0; i < lib.artists.size(); ++i)
  {
    Rectangle r     = {8, y, LEFT_W - 16, 22};
    bool      hover = CheckCollisionPointRec(mouse, r);

    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      lib.selectedArtist = (int)i;

    Color c = (int)i == lib.selectedArtist ? ACCENT : TEXT_MAIN;

    ui::text::drawTruncated(fonts.regular, lib.artists[i].c_str(), {r.x, r.y}, 18, 1,
                            hover ? ACCENT : c, LEFT_W - 24);

    y += ITEM_H;
  }

  EndScissorMode();
}

} // namespace frontend::raylib::view
