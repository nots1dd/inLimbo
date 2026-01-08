#include "frontend/Interface.hpp"

#include "frontend/raylib/Interface.hpp"
#include "query/SongMap.hpp"
#include <algorithm>
#include <raylib.h>

#include "utils/timer/Timer.hpp"

namespace frontend::raylib
{

const char* UI_GLYPHS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
                        "äöüßéèàçñøå"
                        "₹€£¥•–—“”‘’✓✗▶⏸♪ⓘ"
                        "⏮⏭▶⏸"
                        ":;,./?[]{}-=+_()*&^%$#@!`~\\|<>\'\""
                        "…•°±×÷";

static constexpr int WIN_W = 1200;
static constexpr int WIN_H = 700;

static constexpr int HEADER_H = 50;
static constexpr int LEFT_W   = 260;
static constexpr int RIGHT_X  = LEFT_W + 10;
static constexpr int STATUS_H = 48;

static constexpr float ITEM_H = 24.0f;
static constexpr float VIEW_H = WIN_H - STATUS_H - HEADER_H - 48;

static void drawCenteredText(Font font, const char* text, Vector2 center, float fontSize,
                             Color color)
{
  Vector2 size = MeasureTextEx(font, text, fontSize, 1);
  DrawTextEx(font, text, {center.x - size.x * 0.5f, center.y - size.y * 0.5f}, fontSize, 1, color);
}

void Interface::drawTextTruncatedCentered(Font font, const char* text, float centerX, float y,
                                          float fontSize, float spacing, Color color,
                                          float maxWidth)
{
  std::string truncated = truncateText(text, maxWidth, fontSize);
  Vector2     size      = MeasureTextEx(font, truncated.c_str(), fontSize, spacing);

  DrawTextEx(font, truncated.c_str(), {centerX - size.x * 0.5f, y}, fontSize, spacing, color);
}

static void ensureVisible(int index, float itemHeight, float viewHeight, float contentHeight,
                          float& scrollY)
{
  float itemTop    = index * itemHeight;
  float itemBottom = itemTop + itemHeight;

  float viewTop    = -scrollY;
  float viewBottom = viewTop + viewHeight;

  if (itemTop < viewTop)
    scrollY = -itemTop;
  else if (itemBottom > viewBottom)
    scrollY = -(itemBottom - viewHeight);

  float minScroll = std::min(0.0f, viewHeight - contentHeight);
  scrollY         = std::clamp(scrollY, minScroll, 0.0f);
}

static auto smooth(float current, float target, float speed) -> float
{
  return current + (target - current) * speed;
}

auto Interface::truncateText(const char* text, float maxWidth, float fontSize) -> std::string
{
  std::string str(text);
  Vector2     size = MeasureTextEx(m_fontRegular, text, fontSize, 1);

  if (size.x <= maxWidth)
    return str;

  std::string result = str;
  while (result.length() > 0)
  {
    result                   = str.substr(0, result.length() - 1);
    std::string withEllipsis = result + "...";
    size                     = MeasureTextEx(m_fontRegular, withEllipsis.c_str(), fontSize, 1);
    if (size.x <= maxWidth)
      return withEllipsis;
  }

  return "...";
}

void Interface::drawTextTruncated(Font font, const char* text, Vector2 pos, float fontSize,
                                  float spacing, Color color, float maxWidth)
{
  std::string truncated = truncateText(text, maxWidth, fontSize);
  DrawTextEx(font, truncated.c_str(), pos, fontSize, spacing, color);
}

void Interface::playSongWithAlbumQueue(audio::Service& audio, const Song& song)
{
  audio.clearPlaylist();

  auto h = audio.registerTrack(song);
  audio.addToPlaylist(h);
  audio.nextTrack();

  query::songmap::read::forEachSongInAlbum(
    *m_songMapTS, song.metadata.artist, song.metadata.album,
    [&](const Disc&, const Track&, const ino_t, const Song& s) -> void
    {
      if (s.metadata.track <= song.metadata.track)
        return;

      auto next = audio.registerTrack(s);
      audio.addToPlaylist(next);
    });

  if (m_mprisService)
  {
    m_mprisService->updateMetadata();
    m_mprisService->notify();
  }
}

void Interface::run(audio::Service& audio)
{
  InitWindow(WIN_W, WIN_H, "InLimbo Player");
  SetTargetFPS(30);

  int  codepointsCount = 0;
  int* codepoints      = LoadCodepoints(UI_GLYPHS, &codepointsCount);

  m_fontRegular =
    LoadFontEx("assets/fonts/SpaceMonoNerdFont-Regular.ttf", 32, codepoints, codepointsCount);
  m_fontBold =
    LoadFontEx("assets/fonts/SpaceMonoNerdFont-Bold.ttf", 34, codepoints, codepointsCount);

  SetTextureFilter(m_fontRegular.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureFilter(m_fontBold.texture, TEXTURE_FILTER_BILINEAR);

  rebuildArtists();
  rebuildAlbums();

  if (m_mprisService)
    m_mprisService->updateMetadata();

  m_isRunning.store(true);
  m_screen = Screen::Library;

  while (m_isRunning.load() && !WindowShouldClose())
  {
    handleInput(audio);

    BeginDrawing();
    ClearBackground(BG_MAIN);
    draw(audio);
    EndDrawing();

    m_mprisService->poll();
  }

  if (m_artLoaded)
    UnloadTexture(m_artTex);

  UnloadFont(m_fontRegular);
  UnloadFont(m_fontBold);

  CloseWindow();
}

void Interface::rebuildArtists()
{
  m_artists.clear();

  query::songmap::read::forEachArtist(*m_songMapTS,
                                      [&](const Artist& artist, const AlbumMap&) -> void
                                      { m_artists.push_back(artist); });

  m_selArtist = 0;
}

void Interface::rebuildAlbums()
{
  m_albums.clear();
  if (m_artists.empty())
    return;

  const Artist& a = m_artists[m_selArtist];

  query::songmap::read::forEachAlbum(
    *m_songMapTS,
    [&](const Artist& artist, const Album& album, const DiscMap&) -> void
    {
      if (utils::string::isEquals(artist, a))
        m_albums.push_back(album);
    });

  m_selAlbum = 0;
}

void Interface::handleInput(audio::Service& audio)
{
  bool trackChange = false;

  if (IsKeyPressed(KEY_Q))
    m_isRunning.store(false);

  if (IsKeyPressed(KEY_I))
    m_screen = (m_screen == Screen::Library) ? Screen::NowPlaying : Screen::Library;

  if (IsKeyPressed(KEY_EQUAL))
    audio.setVolume(std::min(1.5f, audio.getVolume() + 0.05f));

  if (IsKeyPressed(KEY_MINUS))
    audio.setVolume(std::max(0.0f, audio.getVolume() - 0.05f));

  if (IsKeyPressed(KEY_P))
    audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();

  if (IsKeyPressed(KEY_N))
  {
    trackChange = true;
    audio.nextTrack();
  }

  if (IsKeyPressed(KEY_B))
  {
    trackChange = true;
    audio.previousTrack();
  }

  if (m_screen == Screen::Library)
  {
    if (IsKeyPressed(KEY_UP))
    {
      m_selArtist = std::max(0, m_selArtist - 1);
      rebuildAlbums();
      float contentHeight = m_artists.size() * ITEM_H;

      ensureVisible(m_selArtist, ITEM_H, VIEW_H, contentHeight, m_artistScrollY);
    }

    if (IsKeyPressed(KEY_DOWN))
    {
      m_selArtist = std::min(int(m_artists.size()) - 1, m_selArtist + 1);
      rebuildAlbums();
      float contentHeight = m_artists.size() * ITEM_H;

      ensureVisible(m_selArtist, ITEM_H, VIEW_H, contentHeight, m_artistScrollY);
    }
  }
  if (trackChange && m_mprisService)
    m_mprisService->updateMetadata();

  m_mprisService->notify();
}

void Interface::draw(audio::Service& audio)
{
  if (m_screen == Screen::Library)
  {
    drawHeader("Library");
    drawArtistsPane(audio);
    drawAlbumsPane(audio);
    drawStatusBar(audio);
  }
  else
  {
    drawHeader("Now Playing");
    drawNowPlaying(audio);
  }

  if (m_showMetaInfo && m_screen == Screen::NowPlaying)
  {
    auto meta = audio.getCurrentMetadata();
    if (meta)
    {
      DrawRectangle(0, 0, WIN_W, WIN_H, {0, 0, 0, 120}); // dim background
      drawMetadataOverlay(*meta);
    }
  }

  auto info = audio.getCurrentTrackInfo();
  if (info)
  {
    if (info->lengthSec > 0 && info->positionSec >= info->lengthSec - 0.05f)
    {
      if (!m_trackEndedHandled)
      {
        audio.nextTrack();
        if (m_mprisService)
          m_mprisService->updateMetadata();

        m_trackEndedHandled = true;
      }
    }
    else
    {
      // reset when a new track starts
      m_trackEndedHandled = false;
    }
  }

  /* Fade overlay */
  if (m_screenBlend > 0.01f)
  {
    DrawRectangle(0, 0, WIN_W, WIN_H, {0, 0, 0, (unsigned char)(m_screenBlend * 160)});
  }
}

void Interface::drawHeader(const char* title)
{
  DrawRectangle(0, 0, WIN_W, HEADER_H, BG_PANEL);
  DrawLine(0, HEADER_H, WIN_W, HEADER_H, {60, 60, 60, 255});

  // App title on left
  DrawTextEx(m_fontBold, "InLimbo Player (GUI)", {16, 14}, 22, 1, ACCENT);

  // Screen title on right
  int tw = MeasureTextEx(m_fontRegular, title, 18, 1).x;
  DrawTextEx(m_fontRegular, title, {(float)(WIN_W - tw - 50), 16}, 18, 1, TEXT_DIM);

  constexpr float HEADER_PAD = 16.0f;

  Rectangle infoBtn = {WIN_W - HEADER_PAD - 24, // right aligned with padding
                       (HEADER_H - 24) * 0.5f,  // vertically centered in header
                       24, 24};

  bool hover = CheckCollisionPointRec(GetMousePosition(), infoBtn);

  DrawRectangleRounded(infoBtn, 0.3f, 6, hover ? ACCENT : BG_PANEL);

  drawCenteredText(m_fontRegular, "(i)", {infoBtn.x + 12, infoBtn.y + 12}, 18,
                   hover ? WHITE : TEXT_DIM);

  if (CheckCollisionPointRec(GetMousePosition(), infoBtn) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    m_showMetaInfo = !m_showMetaInfo;
  }
}

void Interface::drawMetadataOverlay(const Metadata& meta)
{
  Rectangle box = {200, 120, WIN_W - 400, WIN_H - 240};

  DrawRectangleRounded(box, 0.05f, 8, BG_PANEL);
  DrawRectangleRoundedLines(box, 0.05f, 8, ACCENT);

  int y = box.y + 20;

  auto line = [&](const std::string& s) -> void
  {
    DrawTextEx(m_fontRegular, s.c_str(), {box.x + 20, (float)y}, 16, 1, TEXT_MAIN);
    y += 22;
  };

  line("Title: " + meta.title);
  line("Artist: " + meta.artist);
  line("Album: " + meta.album);
  line("Genre: " + meta.genre);
  line("Year: " + std::to_string(meta.year));
  line("Track: " + std::to_string(meta.track));
  line("Duration: " + utils::fmtTime(meta.duration));
  line("Path: " + meta.filePath);
}

void Interface::drawArtistsPane(audio::Service& audio)
{
  Rectangle pane = {0, HEADER_H, LEFT_W, WIN_H - STATUS_H - HEADER_H};
  DrawRectangleRec(pane, BG_PANEL);

  // Panel title
  DrawTextEx(m_fontBold, "Artists", {12, HEADER_H + 12}, 18, 1, TEXT_MAIN);
  DrawLine(8, HEADER_H + 38, LEFT_W - 8, HEADER_H + 38, {50, 50, 50, 255});

  Vector2 mouse = GetMousePosition();

  if (CheckCollisionPointRec(mouse, pane))
    m_artistScrollY += GetMouseWheelMove() * 30;

  float contentHeight = m_artists.size() * ITEM_H;
  float viewHeight    = VIEW_H;

  float minScroll = std::min(0.0f, viewHeight - contentHeight);
  m_artistScrollY = std::clamp(m_artistScrollY, minScroll, 0.0f);

  BeginScissorMode(0, HEADER_H + 48, LEFT_W, VIEW_H);

  int y = HEADER_H + 48 + m_artistScrollY;

  for (size_t i = 0; i < m_artists.size(); ++i)
  {
    Rectangle r = {8, (float)y, LEFT_W - 16, 22};

    bool  hover = CheckCollisionPointRec(mouse, r);
    Color c     = (int)i == m_selArtist ? ACCENT : TEXT_MAIN;

    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
      m_selArtist = (int)i;
      rebuildAlbums();
      m_albumScrollY = 0;
    }

    // Truncate artist names
    drawTextTruncated(m_fontRegular, m_artists[i].c_str(), {r.x, r.y}, 18, 1, hover ? ACCENT : c,
                      LEFT_W - 24);
    y += 24;
  }

  EndScissorMode();
}

void Interface::drawAlbumsPane(audio::Service& audio)
{
  if (m_artists.empty())
    return;
  float totalContentHeight = 0.0f;

  const Artist& artist = m_artists[m_selArtist];

  query::songmap::read::forEachAlbum(
    *m_songMapTS,
    [&](const Artist& a, const Album&, const DiscMap& discs) -> void
    {
      if (!utils::string::isEquals(a, artist))
        return;

      totalContentHeight += 30; // album title

      for (const auto& [disc, tracks] : discs)
      {
        totalContentHeight += 20; // disc label

        for (const auto& [_, inodeMap] : tracks)
        {
          totalContentHeight += inodeMap.size() * 18; // tracks
        }

        totalContentHeight += 8; // spacing
      }

      totalContentHeight += 16; // album spacing
    });

  Rectangle pane  = {RIGHT_X, HEADER_H, WIN_W - RIGHT_X, WIN_H - STATUS_H - HEADER_H};
  Vector2   mouse = GetMousePosition();

  if (CheckCollisionPointRec(mouse, pane))
    m_albumScrollY += GetMouseWheelMove() * 30;

  float viewHeight = pane.height - 12;
  float minScroll  = std::min(0.0f, viewHeight - totalContentHeight);
  m_albumScrollY   = std::clamp(m_albumScrollY, minScroll, 0.0f);

  int x = RIGHT_X + 12;
  int y = HEADER_H + 12 + m_albumScrollY;

  BeginScissorMode(pane.x, pane.y, pane.width, pane.height);

  query::songmap::read::forEachAlbum(
    *m_songMapTS,
    [&](const Artist& a, const Album& album, const DiscMap& discs) -> void
    {
      if (!utils::string::isEquals(a, artist))
        return;

      // Truncate album titles
      drawTextTruncated(m_fontBold, album.c_str(), {(float)x, (float)y}, 22, 1, ACCENT,
                        WIN_W - RIGHT_X - 24);
      y += 30;

      for (const auto& [disc, tracks] : discs)
      {
        DrawTextEx(m_fontRegular, TextFormat("Disc %d", disc), {(float)x + 10, (float)y}, 16, 1,
                   TEXT_DIM);
        y += 20;

        for (const auto& [trackNo, inodeMap] : tracks)
        {
          for (const auto& [_, song] : inodeMap)
          {
            Rectangle r     = {(float)x + 28, (float)y, WIN_W - RIGHT_X - 56, 18};
            bool      hover = CheckCollisionPointRec(mouse, r);

            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
              playSongWithAlbumQueue(audio, song);
            }

            // Truncate track titles
            std::string trackText = TextFormat("%02d. %s", trackNo, song.metadata.title.c_str());
            drawTextTruncated(m_fontRegular, trackText.c_str(), {r.x, r.y}, 16, 1,
                              hover ? ACCENT : TEXT_MAIN, WIN_W - RIGHT_X - 70);
            y += 18;
          }
        }
        y += 8;
      }
      y += 16;
    });

  EndScissorMode();
}

void Interface::drawCenteredArt(const Metadata& meta)
{
  if (meta.artUrl != m_loadedArtUrl)
  {
    if (m_artLoaded)
    {
      UnloadTexture(m_artTex);
      m_artLoaded = false;
    }

    m_loadedArtUrl.clear();

    if (!meta.artUrl.empty())
    {
      const char* path =
        meta.artUrl.starts_with("file://") ? meta.artUrl.c_str() + 7 : meta.artUrl.c_str();

      Image img = LoadImage(path);
      if (img.data)
      {
        m_artTex       = LoadTextureFromImage(img);
        m_artLoaded    = true;
        m_loadedArtUrl = meta.artUrl;
        UnloadImage(img);
      }
    }
  }

  if (!m_artLoaded)
  {
    int size = 300;
    int px   = WIN_W / 2 - size / 2;
    int py   = HEADER_H + 40;

    DrawRectangle(px, py, size, size, BG_PANEL);
    DrawRectangleLines(px, py, size, size, {60, 60, 60, 255});
    DrawTextEx(m_fontRegular, "No Album Art", {(float)(px + 80), (float)(py + 140)}, 20, 1,
               TEXT_DIM);
    return;
  }

  float scale = 300.0f / std::max(m_artTex.width, m_artTex.height);
  int   w     = m_artTex.width * scale;

  DrawTextureEx(m_artTex, {(float)(WIN_W / 2 - w / 2), (float)(HEADER_H + 40)}, 0.0f, scale, WHITE);
}

void Interface::drawNowPlaying(audio::Service& audio)
{
  auto meta = audio.getCurrentMetadata();
  auto info = audio.getCurrentTrackInfo();
  if (!meta || !info)
  {
    DrawTextEx(m_fontRegular, "No track playing", {WIN_W / 2.0f - 100, WIN_H / 2.0f}, 20, 1,
               TEXT_DIM);
    return;
  }

  drawCenteredArt(*meta);

  int               textY        = HEADER_H + 360;
  const std::string artistAlbum  = meta->artist + " • " + meta->album;
  float             centerX      = WIN_W * 0.5f;
  float             maxTextWidth = 600.0f;

  drawTextTruncatedCentered(m_fontBold, meta->title.c_str(), centerX, (float)textY, 28, 1,
                            TEXT_MAIN, maxTextWidth);
  drawTextTruncatedCentered(m_fontRegular, artistAlbum.c_str(), centerX, (float)(textY + 40), 18, 1,
                            TEXT_DIM, maxTextWidth);

  drawProgressBar(*info, audio);

  drawPlaybackControls(audio);
}

void Interface::drawPlaybackControls(audio::Service& audio)
{
  float centerX = WIN_W / 2.0f;
  float y       = WIN_H - 60;

  float btnR = 18;

  Rectangle prev = {centerX - 80, y - btnR, btnR * 2, btnR * 2};
  Rectangle play = {centerX - 20, y - btnR, btnR * 2, btnR * 2};
  Rectangle next = {centerX + 40, y - btnR, btnR * 2, btnR * 2};

  Vector2 mouse = GetMousePosition();

  // Prev
  DrawCircle(prev.x + btnR, prev.y + btnR, btnR, BG_PANEL);
  drawCenteredText(m_fontRegular, "<", {prev.x + btnR, prev.y + btnR}, 22, TEXT_MAIN);

  if (CheckCollisionPointRec(mouse, prev) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    audio.previousTrack();
    if (m_mprisService)
      m_mprisService->updateMetadata();
  }

  // Play / Pause
  const char* icon = audio.isPlaying() ? "==" : "!!";

  DrawCircle(play.x + btnR, play.y + btnR, btnR, ACCENT);
  drawCenteredText(m_fontBold, icon, {play.x + btnR, play.y + btnR}, 24, WHITE);

  if (CheckCollisionPointRec(mouse, play) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();
    m_mprisService->notify();
  }

  // Next
  DrawCircle(next.x + btnR, next.y + btnR, btnR, BG_PANEL);
  drawCenteredText(m_fontRegular, ">", {next.x + btnR, next.y + btnR}, 22, TEXT_MAIN);

  if (CheckCollisionPointRec(mouse, next) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    audio.nextTrack();
    if (m_mprisService)
      m_mprisService->updateMetadata();
  }
}

void Interface::drawProgressBar(const audio::service::TrackInfo& info, audio::Service& audio)
{
  float barX = 200;
  float barY = WIN_H - 100;
  float barW = WIN_W - 400;

  static float knob  = barX;
  float        ratio = info.lengthSec > 0 ? info.positionSec / info.lengthSec : 0.0f;

  knob = smooth(knob, barX + ratio * barW, 0.15f);

  std::string posTime = utils::fmtTime(info.positionSec);
  std::string lenTime = utils::fmtTime(info.lengthSec);

  DrawTextEx(m_fontRegular, posTime.c_str(), {barX - 50, barY - 2}, 14, 1, TEXT_DIM);
  DrawTextEx(m_fontRegular, lenTime.c_str(), {barX + barW + 10, barY - 2}, 14, 1, TEXT_DIM);

  DrawRectangle(barX, barY, barW, 6, BG_PANEL);
  DrawRectangle(barX, barY, knob - barX, 6, ACCENT);
  DrawCircle(knob, barY + 3, 8, ACCENT);

  Rectangle r = {barX, barY - 8, barW, 22};
  if (CheckCollisionPointRec(GetMousePosition(), r) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    float t = std::clamp((GetMouseX() - barX) / barW, 0.0f, 1.0f);
    audio.seekAbsolute(t * info.lengthSec);
    if (m_mprisService)
      m_mprisService->notify();
  }
}

void Interface::drawStatusBar(audio::Service& audio)
{
  const int h = STATUS_H;
  DrawRectangle(0, WIN_H - h, WIN_W, h, BG_PANEL);
  auto info = audio.getCurrentTrackInfo();
  auto meta = audio.getCurrentMetadata();
  if (!info || !meta)
    return;

  DrawLine(0, WIN_H - h, WIN_W, WIN_H - h, {60, 60, 60, 255});

  drawTextTruncated(m_fontBold, meta->title.c_str(), {12, (float)WIN_H - h + 6}, 18, 1, TEXT_MAIN,
                    300);

  drawTextTruncated(m_fontRegular, meta->artist.c_str(), {12, (float)WIN_H - h + 26}, 14, 1,
                    TEXT_DIM, 300);

  const std::string time =
    utils::fmtTime(info->positionSec) + " / " + utils::fmtTime(info->lengthSec);
  int tw = MeasureTextEx(m_fontRegular, time.c_str(), 16, 1).x;
  DrawTextEx(m_fontRegular, time.c_str(), {(float)(WIN_W / 2 - tw / 2), (float)WIN_H - h + 16}, 16,
             1, TEXT_DIM);

  const std::string genreText =
    meta->genre.empty() ? "" : ("• " + meta->genre);

  int genreW = MeasureTextEx(m_fontRegular, genreText.c_str(), 14, 1).x;

  std::string right =
    TextFormat("VOL %d%% %s", int(audio.getVolume() * 100),
               audio.isPlaying() ? "==" : "!!");

  int rightW = MeasureTextEx(m_fontRegular, right.c_str(), 16, 1).x;

  float rightX = WIN_W - rightW - 12;
  float genreX = rightX - genreW - 12;

  if (!genreText.empty())
  {
    DrawTextEx(
      m_fontRegular,
      genreText.c_str(),
      {genreX, (float)WIN_H - h + 18},
      14, 1,
      TEXT_DIM
    );
  }

  DrawTextEx(
    m_fontRegular,
    right.c_str(),
    {rightX, (float)WIN_H - h + 16},
    16, 1,
    TEXT_DIM
  );
}

} // namespace frontend::raylib
