#include "frontend/raylib/Interface.hpp"
#include "frontend/raylib/Constants.hpp"
#include "query/SongMap.hpp"
#include <raylib.h>

namespace frontend::raylib
{

static constexpr int WIN_W = 1200;
static constexpr int WIN_H = 700;

void Interface::run(audio::Service& audio)
{
  InitWindow(WIN_W, WIN_H, "InLimbo Player");
  SetTargetFPS(60);

  m_fonts.load();

  query::songmap::read::forEachArtist(*m_songMap, [&](const Artist& artist, const AlbumMap&) -> void
                                      { m_library.artists.push_back(artist); });

  if (m_mpris)
    m_mpris->updateMetadata();

  while (m_ui.running && !WindowShouldClose())
  {
    m_input.handle(audio, m_ui, m_library, m_mpris);

    BeginDrawing();
    ClearBackground(BG_MAIN);

    m_header.draw(m_fonts, m_ui);

    if (m_ui.screen == state::UI::Screen::Library)
    {
      m_artists.draw(m_fonts, m_library, audio);
      m_albums.draw(m_fonts, m_library, audio, *m_songMap, *m_mpris);
      m_status.draw(m_fonts, audio);
    }
    else
    {
      m_nowPlaying.draw(m_fonts, audio, *m_mpris);
    }

    if (m_ui.showMetaInfo && m_ui.screen == state::UI::Screen::NowPlaying)
    {
      auto meta = audio.getCurrentMetadata();
      if (meta)
      {
        DrawRectangle(0, 0, WIN_W, WIN_H, {0, 0, 0, 120}); // dim background
        m_metaOverlay.draw(m_fonts, *meta);
      }
    }

    EndDrawing();

    m_mpris->poll();
  }

  m_fonts.unload();
  CloseWindow();
}

} // namespace frontend::raylib
