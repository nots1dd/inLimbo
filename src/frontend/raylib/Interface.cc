#include "frontend/raylib/Interface.hpp"
#include "Logger.hpp"
#include "config/Config.hpp"
#include "config/sort/Model.hpp"
#include "frontend/raylib/Constants.hpp"
#include "query/SongMap.hpp"

namespace colors = config::colors;

namespace frontend::raylib
{

void Interface::draw(audio::Service& audio)
{
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
}

static auto autoNextIfFinished(audio::Service& audio, mpris::Service& mpris) -> void
{
  static ui8 lastTid;
  auto       infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
    return;

  const auto& info = *infoOpt;

  if (!info.playing)
    return;

  if (info.lengthSec <= 0.0)
    return;

  // we are already in the new song
  if (info.tid == lastTid)
    return;

  constexpr double EPS = 0.10;

  // the next song might have already played but thread didnt catch in time
  // so we just check if position is valid
  if (info.positionSec + EPS < info.lengthSec)
    return;

  lastTid = info.tid;
  audio.nextTrackGapless();
  mpris.updateMetadata();
  mpris.notify();
}

// checks for config watcher status and playback status
// (for gapless next)
//
// note that since we are only reading m_ui.running, it is
// not set to atomic.
void Interface::statusLoop(audio::Service& audio)
{
  while (m_ui.running)
  {
    if (m_cfgWatcher.pollChanged())
    {
      LOG_DEBUG("Configuration file changed, reloading...");
      loadConfig();
    }
    autoNextIfFinished(audio, *m_mpris);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

static constexpr int WIN_W = 1200;
static constexpr int WIN_H = 700;

void Interface::loadConfig()
{
  try
  {
    m_library.artists.clear();
    config::Config::load();

    auto plan = config::sort::loadRuntimeSortPlan();
    m_songMap->update([&](auto& map) -> void { query::sort::applyRuntimeSortPlan(map, plan); });

    colors::ConfigLoader           colorsCfg(FRONTEND_NAME);
    config::keybinds::ConfigLoader keysCfg(FRONTEND_NAME);

    RaylibConfig next;
    next.kb     = Keybinds::load(FRONTEND_NAME);
    next.colors = UiColors::load(FRONTEND_NAME);

    m_cfg.set(std::move(next));

    LOG_INFO("Configuration loaded for {}'s keybinds and colors, and song map sort plans.",
             FRONTEND_NAME);

    query::songmap::read::forEachArtist(*m_songMap,
                                        [&](const Artist& artist, const AlbumMap&) -> void
                                        { m_library.artists.push_back(artist); });
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
  m_ui.running = true;

  if (m_mpris)
    m_mpris->updateMetadata();

  std::thread status([&]() -> void { statusLoop(audio); });

  while (m_ui.running && !WindowShouldClose())
  {
    auto cfg = m_cfg.get();
    m_input.handle(audio, m_ui, *cfg, m_mpris);
    draw(audio);
    m_mpris->poll();
  }

  m_fonts.unload();
  if (status.joinable())
    status.join();

  CloseWindow();
}

} // namespace frontend::raylib
