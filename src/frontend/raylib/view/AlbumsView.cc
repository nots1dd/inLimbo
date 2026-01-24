#include "frontend/raylib/view/AlbumsView.hpp"
#include "frontend/raylib/Constants.hpp"
#include "frontend/raylib/ui/TextUtils.hpp"
#include "query/SongMap.hpp"
#include "utils/string/Equals.hpp"
#include "utils/timer/Timer.hpp"

namespace frontend::raylib::view
{

static void playSongWithAlbumQueue(audio::Service& audio, threads::SafeMap<SongMap>& songs,
                                   const Song& song, mpris::Service& mpris)
{
  audio.clearPlaylist();

  auto h = audio.registerTrack(song);
  audio.addToPlaylist(h);
  audio.nextTrack();

  query::songmap::read::forEachSongInAlbum(
    songs, song.metadata.artist, song.metadata.album,
    [&](const Disc&, const Track&, const ino_t, const Song& s) -> void
    {
      if (s.metadata.track <= song.metadata.track)
        return;

      auto next = audio.registerTrack(s);
      audio.addToPlaylist(next);
    });

  mpris.updateMetadata();
}

static void drawHoverRow(const Rectangle& r, bool hover)
{
  if (!hover)
    return;

  DrawRectangleRounded({r.x - 6, r.y - 2, r.width + 12, r.height + 4}, 0.2f, 6,
                       {255, 255, 255, 20});
}

static auto getContentHeight(threads::SafeMap<SongMap>& songs, const Artist& artist) -> float
{
  float h = 0.0f;

  query::songmap::read::forEachAlbum(
    songs,
    [&](const Artist& a, const Album&, const DiscMap& discs) -> void
    {
      if (!utils::string::isEquals(a, artist))
        return;

      h += 36; // album header

      for (auto& [disc, tracks] : discs)
      {
        h += 18; // disc header

        for (auto& [_, inodeMap] : tracks)
        {
          h += inodeMap.size() * 22; // tracks
        }

        h += 12; // gap after disc
      }

      h += 20; // gap after album
    });

  return h;
}

static constexpr int RIGHT_X = LEFT_W + 10;

void AlbumsView::draw(const ui::Fonts& fonts, state::Library& lib, audio::Service& audio,
                      threads::SafeMap<SongMap>& songs, mpris::Service& mpris)
{
  if (lib.artists.empty())
    return;

  Rectangle pane  = {RIGHT_X, HEADER_H, 1200 - RIGHT_X, 700 - HEADER_H - 48};
  Vector2   mouse = GetMousePosition();

  const Artist& artist = lib.artists[lib.selectedArtist];

  const auto contentHeight = getContentHeight(songs, artist);

  if (CheckCollisionPointRec(mouse, pane))
  {
    lib.albumScrollY += GetMouseWheelMove() * 30;

    float viewHeight = pane.height;
    float minScroll  = std::min(0.0f, viewHeight - contentHeight);

    lib.albumScrollY = std::clamp(lib.albumScrollY, minScroll, 0.0f);
  }

  BeginScissorMode(pane.x, pane.y, pane.width, pane.height);

  int x = RIGHT_X + 20;
  int y = HEADER_H + 16 + lib.albumScrollY;

  query::songmap::read::forEachAlbum(
    songs,
    [&](const Artist& a, const Album& album, const DiscMap& discs) -> void
    {
      if (!utils::string::isEquals(a, artist))
        return;

      /* ---------------- Album header ---------------- */
      ui::text::drawTruncated(fonts.bold, album.c_str(), {(float)x, (float)y}, 22, 1, TEXT_MAIN,
                              1200 - RIGHT_X - 40);

      // Accent underline
      DrawRectangle(x, y + 26, 48, 2, ACCENT);

      y += 36;

      /* ---------------- Discs ---------------- */
      for (auto& [disc, tracks] : discs)
      {
        DrawTextEx(fonts.regular, TextFormat("Disc %d", disc), {(float)x + 4, (float)y}, 14, 1,
                   TEXT_DIM);

        y += 18;

        /* ---------------- Tracks ---------------- */
        for (auto& [_, inodeMap] : tracks)
        {
          for (auto& [__, song] : inodeMap)
          {
            Rectangle row = {(float)x, (float)y, pane.width - 40, 20};

            bool hover = CheckCollisionPointRec(mouse, row);
            drawHoverRow(row, hover);

            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
              playSongWithAlbumQueue(audio, songs, song, mpris);

            // Track title (left)
            std::string title =
              TextFormat("%02d  %s", song.metadata.track, song.metadata.title.c_str());

            ui::text::drawTruncated(fonts.regular, title.c_str(), {row.x + 8, row.y + 2}, 16, 1,
                                    hover ? ACCENT : TEXT_MAIN, row.width - 80);

            // Duration (right)
            std::string dur = utils::fmtTime(song.metadata.duration);

            int dw = MeasureTextEx(fonts.regular, dur.c_str(), 14, 1).x;

            DrawTextEx(fonts.regular, dur.c_str(), {row.x + row.width - dw - 8, row.y + 2}, 14, 1,
                       TEXT_DIM);

            y += 22;
          }
        }

        y += 12;
      }

      y += 20;
    });

  EndScissorMode();
}

} // namespace frontend::raylib::view
