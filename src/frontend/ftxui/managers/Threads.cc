#include "frontend/ftxui/managers/Threads.hpp"
#include "config/Misc.hpp"
#include "config/sort/Model.hpp"
#include "helpers/telemetry/Playback.hpp"
#include "query/SongMap.hpp"
#include "utils/fs/Paths.hpp"
#include <chrono>

namespace frontend::tui::managers
{

static auto autoNextIfFinished(audio::Service& audio, mpris::Service& mpris,
                               std::atomic<ui8>& lastTid, std::atomic<bool>& inProgress) -> bool
{
  if (inProgress.load(std::memory_order_acquire))
    return false;

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
    return false;

  const auto& info = *infoOpt;

  if (info.lengthSec <= 0.0)
    return false;

  constexpr double EPS = 0.05;
  if (info.positionSec + EPS < info.lengthSec)
    return false;

  if (info.tid == lastTid.load(std::memory_order_relaxed))
    return false;

  bool expected = false;
  if (!inProgress.compare_exchange_strong(expected, true))
    return false;

  lastTid.store(info.tid, std::memory_order_relaxed);

  audio.nextTrackGapless();
  mpris.updateMetadata();
  mpris.notify();

  inProgress.store(false, std::memory_order_release);
  return true;
}

void ThreadManager::executeWithTelemetry(const std::function<void(audio::Service&)>& fn)
{
  if (!m_audioPtr || !m_telemetry)
    return;

  helpers::telemetry::playbackTransition(*m_audioPtr, m_telemetry, currentPlay, lastPlayTick,
                                         [&]() -> void { fn(*m_audioPtr); });
}

void ThreadManager::setOnConfigReload(std::function<void()> fn)
{
  m_onConfigReload = std::move(fn);
}

void ThreadManager::loadMiscConfig(MiscConfig& miscCfg)
{
  config::misc::ConfigLoader loader(FRONTEND_NAME);

  loader.load(
    config::keybinds::Binding<int>{.key = "seek_duration", .target = &miscCfg.seekDuration});
}

void ThreadManager::loadConfig()
{
  try
  {
    tomlparser::Config::load();

    auto plan = config::sort::loadRuntimeSortPlan();
    m_songMap->withWriteLock([&](auto& map) -> void
                             { query::sort::applyRuntimeSortPlan(map, plan); });

    config::colors::ConfigLoader   colorsCfg(FRONTEND_NAME);
    config::keybinds::ConfigLoader keysCfg(FRONTEND_NAME);

    TuiConfig next;
    next.kb     = Keybinds::load(FRONTEND_NAME);
    next.colors = UiColors::load(FRONTEND_NAME);

    loadMiscConfig(next.misc);

    m_cfg.set(std::move(next));

    LOG_INFO("Configuration loaded for {}'s keybinds and colors.", FRONTEND_NAME);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Interface::loadConfig failed: {}", e.what());
  }
}

ThreadManager::ThreadManager(mpris::Service* mpris, threads::SafeMap<SongMap>* songMap,
                             telemetry::Context* telemetry)
    : m_mpris(mpris), m_songMap(songMap), m_telemetry(telemetry),
      m_cfgWatcher(utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME))
{
}

void ThreadManager::start()
{
  running = true;

  helpers::telemetry::beginPlayback(*m_audioPtr, m_telemetry, currentPlay, lastPlayTick);

  status_thread = std::thread(&ThreadManager::statusLoop, this);

  mpris_thread = std::thread(&ThreadManager::mprisLoop, this);

  seek_thread = std::thread(&ThreadManager::seekLoop, this);
}

void ThreadManager::stop()
{
  running = false;

  if (status_thread.joinable())
    status_thread.join();
  if (mpris_thread.joinable())
    mpris_thread.join();
  if (seek_thread.joinable())
    seek_thread.join();

  helpers::telemetry::endPlayback(*m_audioPtr, m_telemetry, currentPlay, lastPlayTick);
}

void ThreadManager::setScreen(ftxui::ScreenInteractive* screen) { m_screen = screen; }

void ThreadManager::statusLoop()
{
  while (running.load())
  {
    helpers::telemetry::updateTelemetryProgress(*m_audioPtr, currentPlay, lastPlayTick);

    if (m_cfgWatcher.pollChanged())
    {
      LOG_DEBUG("Configuration file changed, reloading...");
      loadConfig();

      if (m_onConfigReload)
        m_onConfigReload();

      const auto title = m_audioPtr->getCurrentMetadata()->title;
      m_audioPtr->clearPlaylist();
      auto songObj = query::songmap::read::findSongObjByTitle(*m_songMap, title);
      auto h       = m_audioPtr->registerTrack(songObj);
      m_audioPtr->addToPlaylist(h);

      m_mpris->updateMetadata();

      query::songmap::read::forEachSongInAlbum(
        *m_songMap, m_audioPtr->getCurrentMetadata()->artist,
        m_audioPtr->getCurrentMetadata()->album,
        [&](const Disc&, const Track&, const ino_t, const std::shared_ptr<Song>& song) -> void
        {
          if (song->metadata.track <= m_audioPtr->getCurrentMetadata()->track)
            return;

          auto h = m_audioPtr->registerTrack(song);
          m_audioPtr->addToPlaylist(h);
        });
    }

    if (auto info = m_audioPtr->getCurrentTrackInfo())
    {
      if (info->lengthSec > 0 && info->positionSec >= info->lengthSec)
      {
        helpers::telemetry::playbackTransition(*m_audioPtr, m_telemetry, currentPlay, lastPlayTick,
                                               [&] { m_audioPtr->nextTrack(); });

        m_mpris->updateMetadata();
        m_mpris->notify();
      }
    }

    if (autoNextIfFinished(*m_audioPtr, *m_mpris, m_lastAutoNextTid, m_autoNextInProgress))
    {
      helpers::telemetry::playbackTransition(
        *m_audioPtr, m_telemetry, currentPlay, lastPlayTick,
        [] { /* already transitioned in autoNextIfFinished */ });
    }

    if (m_screen)
      m_screen->PostEvent(ftxui::Event::Custom);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void ThreadManager::mprisLoop()
{
  while (running.load())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    m_mpris->poll();
  }
}

void ThreadManager::seekLoop()
{
  while (running.load())
  {
    double d = pendingSeek.exchange(0.0);

    if (std::abs(d) > 0.01)
    {
      if (d > 0.0)
        m_audioPtr->seekForward(d);
      else
        m_audioPtr->seekBackward(-d);
    }

    m_mpris->notify();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

} // namespace frontend::tui::managers
