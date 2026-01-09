#include "frontend/raylib/view/MetadataOverlay.hpp"
#include "frontend/raylib/Constants.hpp"
#include "frontend/raylib/ui/TextUtils.hpp"
#include "utils/timer/Timer.hpp"
#include <raylib.h>

namespace frontend::raylib::view
{

void MetadataOverlay::draw(const ui::Fonts& fonts, const Metadata& meta)
{
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 140});

  Rectangle box = {220, 140, 760, 420};

  // Shadow
  DrawRectangleRounded({box.x + 6, box.y + 6, box.width, box.height}, 0.08f, 12, {0, 0, 0, 120});

  // Panel
  DrawRectangleRounded(box, 0.08f, 12, BG_PANEL);
  DrawRectangleRoundedLines(box, 0.08f, 12, ACCENT);

  DrawTextEx(fonts.bold, "Track Information", {box.x + 24, box.y + 20}, 22, 1, TEXT_MAIN);

  DrawLine(box.x + 24, box.y + 52, box.x + box.width - 24, box.y + 52, {60, 60, 60, 255});

  float labelX = box.x + 24;
  float valueX = box.x + 180;
  float y      = box.y + 72;

  auto row = [&](const char* label, const std::string& value) -> void
  {
    DrawTextEx(fonts.regular, label, {labelX, y}, 15, 1, TEXT_DIM);

    DrawTextEx(fonts.regular, value.c_str(), {valueX, y}, 16, 1, TEXT_MAIN);

    y += 28;
  };

  row("Title", meta.title);
  row("Artist", meta.artist);
  row("Album", meta.album);

  // subtle section gap
  y += 8;
  DrawLine(labelX, y, box.x + box.width - 24, y, {50, 50, 50, 255});
  y += 16;

  row("Genre", meta.genre.empty() ? "—" : meta.genre);
  row("Year", std::to_string(meta.year));
  row("Track #", std::to_string(meta.track));
  row("Duration", utils::fmtTime(meta.duration));

  // path gets its own area
  y += 10;
  DrawLine(labelX, y, box.x + box.width - 24, y, {50, 50, 50, 255});
  y += 16;

  DrawTextEx(fonts.regular, "File", {labelX, y}, 15, 1, TEXT_DIM);

  // path wrapped / truncated visually by width
  Rectangle pathRect = {valueX, y, box.width - (valueX - box.x) - 24, 48};

  ui::text::drawWrappedText(fonts.regular, meta.filePath.c_str(), pathRect, 14, 1, TEXT_MAIN);

  const char* hint = "Press (i) to close";
  int         hw   = MeasureTextEx(fonts.regular, hint, 14, 1).x;

  DrawTextEx(fonts.regular, hint, {box.x + box.width - hw - 24, box.y + box.height - 24}, 14, 1,
             TEXT_DIM);
}

} // namespace frontend::raylib::view
