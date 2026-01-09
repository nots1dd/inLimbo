#include "frontend/raylib/view/StatusBar.hpp"
#include "frontend/raylib/Constants.hpp"
#include "frontend/raylib/ui/TextUtils.hpp"
#include "utils/timer/Timer.hpp"

namespace frontend::raylib::view
{

static constexpr int WIN_W    = 1200;
static constexpr int WIN_H    = 700;
static constexpr int STATUS_H = 48;

void StatusBar::draw(const ui::Fonts& fonts, audio::Service& audio)
{
  DrawRectangle(0, WIN_H - STATUS_H, WIN_W, STATUS_H, BG_PANEL);
  DrawLine(0, WIN_H - STATUS_H, WIN_W, WIN_H - STATUS_H, {60, 60, 60, 255});

  auto meta = audio.getCurrentMetadata();
  auto info = audio.getCurrentTrackInfo();
  if (!meta || !info)
    return;

  ui::text::drawTruncated(fonts.bold, meta->title.c_str(), {12, WIN_H - STATUS_H + 6}, 18, 1,
                          TEXT_MAIN, 300);

  ui::text::drawTruncated(fonts.regular, meta->artist.c_str(), {12, WIN_H - STATUS_H + 26}, 14, 1,
                          TEXT_DIM, 300);

  const std::string time =
    utils::fmtTime(info->positionSec) + " / " + utils::fmtTime(info->lengthSec);
  int tw = MeasureTextEx(fonts.regular, time.c_str(), 16, 1).x;
  DrawTextEx(fonts.regular, time.c_str(),
             {(float)(WIN_W / 2 - tw / 2), (float)WIN_H - STATUS_H + 16}, 16, 1, TEXT_DIM);

  const Genre genre = meta->genre.empty() ? "" : ("• " + meta->genre);

  int genreW = MeasureTextEx(fonts.regular, genre.c_str(), 14, 1).x;

  const std::string right =
    TextFormat("VOL %d%% %s", int(audio.getVolume() * 100), audio.isPlaying() ? "==" : "!!");

  int rightW = MeasureTextEx(fonts.regular, right.c_str(), 16, 1).x;

  float rightX = WIN_W - rightW - 12;
  float genreX = rightX - genreW - 12;

  if (!genre.empty())
  {
    DrawTextEx(fonts.regular, genre.c_str(), {genreX, (float)WIN_H - STATUS_H + 18}, 14, 1,
               TEXT_DIM);
  }

  DrawTextEx(fonts.regular, right.c_str(), {rightX, (float)WIN_H - STATUS_H + 16}, 16, 1, TEXT_DIM);
}

} // namespace frontend::raylib::view
