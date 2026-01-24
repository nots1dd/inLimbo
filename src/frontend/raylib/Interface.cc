#include "frontend/raylib/Interface.hpp"
#include "Logger.hpp"
#include "config/colors/ConfigLoader.hpp"
#include "config/keybinds/ConfigLoader.hpp"
#include "frontend/raylib/Constants.hpp"
#include "query/SongMap.hpp"
#include "toml/Parser.hpp"

namespace colors = config::colors;

namespace frontend::raylib
{

static constexpr int WIN_W = 1200;
static constexpr int WIN_H = 700;

void Interface::loadConfig()
{
  try
  {
    tomlparser::Config::load();

    colors::ConfigLoader colorsCfg(FRONTEND_NAME);
    colorsCfg.loadIntoRegistry(true);

    config::keybinds::ConfigLoader keysCfg(FRONTEND_NAME);
    keysCfg.loadIntoRegistry(true);

    RaylibConfig next;
    next.kb     = Keybinds::load(FRONTEND_NAME);
    next.colors = UiColors::load(FRONTEND_NAME);

    m_cfg.set(std::move(next));

    LOG_INFO("Configuration loaded.");
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("loadConfig failed: {}", e.what());
  }
}

void Interface::run(audio::Service& audio)
{
  InitWindow(WIN_W, WIN_H, "InLimbo Player");
  SetTargetFPS(60);

  loadConfig();
  m_fonts.load();

  query::songmap::read::forEachArtist(*m_songMap, [&](const Artist& artist, const AlbumMap&) -> void
                                      { m_library.artists.push_back(artist); });

  if (m_mpris)
    m_mpris->updateMetadata();

  while (m_ui.running && !WindowShouldClose())
  {

    if (m_cfgWatcher.pollChanged())
    {
      LOG_DEBUG("Configuration file changed, reloading...");
      loadConfig();
    }

    auto cfg = m_cfg.get();
    m_input.handle(audio, m_ui, *cfg, m_mpris);

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
