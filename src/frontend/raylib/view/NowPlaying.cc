#include "frontend/raylib/view/NowPlaying.hpp"
#include "frontend/raylib/Constants.hpp"
#include "frontend/raylib/ui/Fonts.hpp"
#include "frontend/raylib/ui/TextUtils.hpp"
#include "frontend/raylib/util/Animation.hpp"
#include "utils/timer/Timer.hpp"
#include <raylib.h>

namespace frontend::raylib::view
{

static void drawWaveformBackground(Rectangle area,
                                   float phase,
                                   float intensity,
                                   Color color)
{
  constexpr int BARS = 48;
  float barW = area.width / BARS;

  for (int i = 0; i < BARS; ++i)
  {
    float t = phase + i * 0.35f;

    float h =
      (0.4f + 0.6f * sinf(t)) *
      intensity *
      area.height;

    Rectangle bar = {
      area.x + i * barW,
      area.y + area.height * 0.5f - h * 0.5f,
      barW * 0.6f,
      h
    };

    DrawRectangleRounded(bar, 0.4f, 4, color);
  }
}

void NowPlaying::draw(const ui::Fonts& fonts, audio::Service& audio, mpris::Service& mpris)
{
  auto meta = audio.getCurrentMetadata();
  auto info = audio.getCurrentTrackInfo();
  if (!meta || !info)
    return;

  drawArt(*meta);

  float cx = WIN_W * 0.5f;

  ui::text::drawCentered(
    fonts.bold,
    meta->title.c_str(),
    {cx, HEADER_H + 370},
    30,
    TEXT_MAIN
  );

  ui::text::drawCentered(
    fonts.regular,
    (meta->artist + " • " + meta->album).c_str(),
    {cx, HEADER_H + 410},
    18,
    TEXT_DIM
  );

  drawProgress(*info, audio, fonts);
  drawControls(audio, fonts, mpris);
}

void NowPlaying::drawArt(const Metadata& meta)
{
  Texture2D* tex = m_art.get(meta);
  if (!tex)
    return;

  constexpr float ART_SIZE = 300.0f;

  float scale = ART_SIZE / std::max(tex->width, tex->height);
  float w     = tex->width * scale;
  float h     = tex->height * scale;

  float x = WIN_W * 0.5f - w * 0.5f;
  float y = HEADER_H + 40;

  DrawRectangleRounded(
    {x - 8, y - 8, w + 16, h + 16},
    0.08f, 12, {0, 0, 0, 140});

  DrawRectangleRounded(
    {x - 4, y - 4, w + 8, h + 8},
    0.08f, 12, BG_PANEL);

  DrawTextureEx(*tex, {x, y}, 0.0f, scale, WHITE);
}

void NowPlaying::drawProgress(const audio::service::TrackInfo& info,
                              audio::Service& audio,
                              const ui::Fonts& fonts)
{
  float barX = 260;
  float barW = WIN_W - 520;
  float barY = WIN_H - 120;

  static float knob      = barX;
  static float knobTarget = barX;

  float ratio = info.lengthSec > 0
                  ? info.positionSec / info.lengthSec
                  : 0.f;

  knobTarget = barX + ratio * barW;

  knob = util::smooth(knob, knobTarget, 0.18f);

  const std::string cur = utils::fmtTime(info.positionSec);
  const std::string len = utils::fmtTime(info.lengthSec);

  DrawTextEx(
    fonts.regular,
    cur.c_str(),
    {barX - 60, barY - 6},
    14, 1, TEXT_DIM
  );

  DrawTextEx(
    fonts.regular,
    len.c_str(),
    {barX + barW + 12, barY - 6},
    14, 1, TEXT_DIM
  );

  Rectangle hit = {barX, barY - 10, barW, 26};
  bool hover = CheckCollisionPointRec(GetMousePosition(), hit);

  // ---- Waveform background ----
  static float wavePhase = 0.0f;
  if (audio.isPlaying())
    wavePhase += GetFrameTime() * 6.0f;

  drawWaveformBackground(
    hit,
    wavePhase,
    audio.isPlaying() ? 1.0f : 0.25f,
    {ACCENT.r, ACCENT.g, ACCENT.b, 40}
  );

  DrawRectangle(barX, barY, barW, 6, BG_PANEL);
  DrawRectangle(barX, barY, knob - barX, 6, ACCENT);

  DrawCircle(knob, barY + 3, hover ? 10 : 8,
             {ACCENT.r, ACCENT.g, ACCENT.b, 120});
  DrawCircle(knob, barY + 3, 6, ACCENT);

  if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    float t = std::clamp((GetMouseX() - barX) / barW, 0.0f, 1.0f);
    knobTarget = barX + t * barW;
    audio.seekAbsolute(t * info.lengthSec);
  }
}

void NowPlaying::drawControls(audio::Service& audio, const ui::Fonts& fonts, mpris::Service& mpris)
{
  float cx = WIN_W / 2.0f;
  float y  = WIN_H - 60;

  Vector2 mouse = GetMousePosition();

  auto drawBtn = [&](Rectangle r, const char* icon, bool active) -> bool
  {
    bool hover = CheckCollisionPointRec(mouse, r);
    float scale = hover ? 1.1f : 1.0f;

    float radius = (r.width * 0.5f) * scale;
    Vector2 c = {r.x + r.width * 0.5f, r.y + r.height * 0.5f};

    DrawCircle(c.x, c.y, radius, active ? ACCENT : BG_PANEL);

    ui::text::drawCentered(
      active ? fonts.bold : fonts.regular,
      icon,
      c,
      active ? 26 : 22,
      active ? WHITE : TEXT_MAIN
    );

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  };

  Rectangle prev = {cx - 96, y - 18, 36, 36};
  Rectangle play = {cx - 24, y - 22, 44, 44};
  Rectangle next = {cx + 52, y - 18, 36, 36};

  bool trackChanged = false;

  if (drawBtn(prev, "<", false))
  {
    audio.previousTrack();
    trackChanged = true;
  }

  const char* icon = audio.isPlaying() ? "==" : "!!";
  if (drawBtn(play, icon, true))
    audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();

  if (drawBtn(next, ">", false))
  {
    audio.nextTrack();
    trackChanged = true;
  }

  if (trackChanged)
    mpris.updateMetadata();
}

} // namespace frontend::raylib::view
