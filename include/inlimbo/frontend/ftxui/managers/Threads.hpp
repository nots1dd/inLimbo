#pragma once

#include "audio/Service.hpp"
#include "config/Watcher.hpp"
#include "frontend/ftxui/Structs.hpp"
#include "mpris/Service.hpp"
#include "telemetry/Context.hpp"
#include "utils/Snapshot.hpp"
#include <atomic>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>

// within UI, this class holds the auto next logic, telemtry storage and updating,
// and the status/seek threading logic.

namespace frontend::tui::managers
{

static inline constexpr cstr FRONTEND_NAME = "ftxui";

class ThreadManager
{
public:
  ThreadManager(mpris::Service* mpris, threads::SafeMap<SongMap>* songMap,
                telemetry::Context* telemetry);

  void start();
  void stop();

  void attachAudioService(audio::Service& audio) { m_audioPtr = &audio; }

  void requestSeek(double positionSec)
  {
    m_pendingSeek.store(positionSec, std::memory_order_release);
  }

  // for us to post screen events on
  void setScreen(ftxui::ScreenInteractive* screen);

  auto getPendingSeek() -> double { return m_pendingSeek.load(std::memory_order_acquire); }

  void executeWithTelemetry(const std::function<void(audio::Service&)>& fn);
  void setOnConfigReload(std::function<void()> fn);

private:
  void loadMiscConfig(MiscConfig& miscCfg);
  void loadConfig();

  void statusLoop();
  void mprisLoop();
  void seekLoop();

  audio::Service*            m_audioPtr;
  mpris::Service*            m_mpris;
  threads::SafeMap<SongMap>* m_songMap{nullptr};
  telemetry::Context*        m_telemetry{nullptr};
  ftxui::ScreenInteractive*  m_screen{nullptr};

  config::Watcher            m_cfgWatcher;
  utils::Snapshot<TuiConfig> m_cfg{};
  std::function<void()>      m_onConfigReload;

  std::thread status_thread;
  std::thread mpris_thread;
  std::thread seek_thread;

  std::atomic<bool>   m_isRunning{false};
  std::atomic<double> m_pendingSeek{0.0};

  std::optional<telemetry::Event> m_currentPlay;
  std::optional<i64>              m_lastPlayTick;

  std::atomic<bool> m_autoNextInProgress{false};
};

} // namespace frontend::tui::managers
