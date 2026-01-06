#include "frontend/Interface.hpp"

#include "frontend/raylib/Interface.hpp"
#include "query/SongMap.hpp"
#include <algorithm>
#include <raylib.h>

#include "utils/timer/Timer.hpp"

INLIMBO_DEFINE_FRONTEND_INTERFACE(raylib)

namespace frontend::raylib
{

static constexpr int WIN_W = 1100;
static constexpr int WIN_H = 650;

static constexpr int LEFT_W  = 260;
static constexpr int RIGHT_X = LEFT_W + 10;

Interface::Interface(threads::SafeMap<SongMap>& songMap, mpris::Service* mprisService)
    : m_songMapTS(songMap), m_mprisService(mprisService)
{
}

void Interface::run(audio::Service& audio)
{

  InitWindow(WIN_W, WIN_H, "InLimbo Player");
  SetTargetFPS(60);

  rebuildArtists();
  rebuildAlbums();

  m_mprisService->updateMetadata();

  m_isRunning.store(true);

  while (m_isRunning.load() && !WindowShouldClose())
  {
    handleInput(audio);

    BeginDrawing();
    ClearBackground({20, 20, 20, 255});

    draw(audio);

    EndDrawing();

    if (m_mprisService)
      m_mprisService->poll();
  }

  if (m_artLoaded)
    UnloadTexture(m_artTex);

  CloseWindow();
}

void Interface::rebuildArtists()
{
  m_artists.clear();

  query::songmap::read::forEachArtist(m_songMapTS,
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
    m_songMapTS,
    [&](const Artist& artist, const Album& album, const DiscMap&) -> void
    {
      if (utils::string::isEquals(artist, a))
        m_albums.push_back(album);
    });

  m_selAlbum = 0;
}

void Interface::handleInput(audio::Service& audio)
{
  bool trackChanged = false;

  if (IsKeyPressed(KEY_Q))
    m_isRunning.store(false);

  if (IsKeyPressed(KEY_I))
    m_showArt = !m_showArt;

  if (IsKeyPressed(KEY_P))
  {
    audio.playCurrent();
    m_dirty = true;
  }
  if (IsKeyPressed(KEY_S))
  {
    audio.pauseCurrent();
    m_dirty = true;
  }

  if (IsKeyPressed(KEY_N))
  {
    audio.nextTrack();
    trackChanged = true;
  }
  if (IsKeyPressed(KEY_B))
  {
    audio.previousTrack();
    trackChanged = true;
  }

  if (IsKeyPressed(KEY_UP))
  {
    m_selArtist = std::max(0, m_selArtist - 1);
    rebuildAlbums();
  }

  if (IsKeyPressed(KEY_DOWN))
  {
    m_selArtist = std::min(int(m_artists.size()) - 1, m_selArtist + 1);
    rebuildAlbums();
  }

  if (IsKeyPressed(KEY_RIGHT))
    audio.seekForward(2.0);

  if (IsKeyPressed(KEY_LEFT))
    audio.seekBackward(2.0);

  if (m_mprisService && (trackChanged || m_dirty))
  {
    if (trackChanged)
      m_mprisService->updateMetadata();

    m_mprisService->notify();
    m_dirty = false;
  }
}

void Interface::draw(audio::Service& audio)
{
  drawArtistsPane();
  drawAlbumsPane(audio);
  drawStatusBar(audio);
}

void Interface::drawArtistsPane()
{
  DrawRectangle(0, 0, LEFT_W, WIN_H, {30, 30, 30, 255});

  int y = 20;
  for (size_t i = 0; i < m_artists.size(); ++i)
  {
    Color c = (int(i) == m_selArtist) ? SKYBLUE : RAYWHITE;
    DrawText(m_artists[i].c_str(), 10, y, 18, c);
    y += 22;
  }
}

void Interface::drawAlbumsPane(audio::Service& audio)
{
  if (m_artists.empty() || m_albums.empty())
    return;

  const Artist& artist = m_artists[m_selArtist];
  const Album&  album  = m_albums[m_selAlbum];

  int x = RIGHT_X;
  int y = 20;

  DrawText(album.c_str(), x, y, 22, GOLD);
  y += 30;

  query::songmap::read::forEachDisc(
    m_songMapTS,
    [&](const Artist& a, const Album& al, Disc disc, const TrackMap& tracks) -> void
    {
      if (!utils::string::isEquals(a, artist) || !utils::string::isEquals(al, album))
        return;

      DrawText(TextFormat("Disc %d", disc), x, y, 16, LIGHTGRAY);
      y += 20;

      for (const auto& [trackNo, inodeMap] : tracks)
      {
        for (const auto& [_, song] : inodeMap)
        {
          DrawText(TextFormat("%02d. %s", trackNo, song.metadata.title.c_str()), x + 20, y, 16,
                   RAYWHITE);
          y += 18;
        }
      }

      y += 10;
    });

  if (m_showArt)
  {
    auto meta = audio.getCurrentMetadata();
    if (meta)
      drawArt(*meta);
  }
}

void Interface::drawArt(const Metadata& meta)
{
  if (meta.artUrl.empty())
    return;

  if (!m_artLoaded)
  {
    // file://
    const std::string filterPath = meta.artUrl.substr(7);
    Image             img        = LoadImage(filterPath.c_str());
    if (img.data)
    {
      m_artTex = LoadTextureFromImage(img);
      UnloadImage(img);
      m_artLoaded = true;
    }
  }

  if (m_artLoaded)
    DrawTextureEx(m_artTex, {750, 60}, 0.0f, 0.4f, WHITE);
}

void Interface::drawStatusBar(audio::Service& audio)
{
  DrawRectangle(0, WIN_H - 40, WIN_W, 40, {25, 25, 25, 255});

  auto info = audio.getCurrentTrackInfo();
  if (!info)
    return;

  DrawText(TextFormat("Time %s / %s | Volume %d%% | [I] Art",
                      utils::fmtTime(info->positionSec).c_str(),
                      utils::fmtTime(info->lengthSec).c_str(), int(audio.getVolume() * 100)),
           10, WIN_H - 28, 18, RAYWHITE);
}

} // namespace frontend::raylib
