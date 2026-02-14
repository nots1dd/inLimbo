#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <ftxui/component/screen_interactive.hpp>

#include "audio/Service.hpp"
#include "config/Watcher.hpp"
#include "helpers/telemetry/Playback.hpp"
#include "mpris/Service.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"
#include "utils/ClassRulesMacros.hpp"
#include "utils/fs/Paths.hpp"

#include "TUI.hpp"

static constexpr cstr FRONTEND_NAME = "tui";

namespace frontend::tui
{

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>* songMap, telemetry::Context* telemetry,
                     mpris::Service* mprisService)
      : m_cfgWatcher(utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME)),
        m_songMapTS(songMap), m_telemetryCtx(telemetry), m_mprisService(mprisService)
  {
  }

  IMMUTABLE(Interface);

  [[nodiscard]] auto ready() -> bool { return m_songMapTS != nullptr; }

  void run(audio::Service& audio)
  {
    if (!ready())
      return;

    m_renderer = std::make_unique<TuiRenderer>(m_songMapTS, audio);

    m_isRunning = true;

    helpers::telemetry::beginPlayback(audio, m_telemetryCtx, m_currentPlay, m_lastPlayTick);

    m_mprisService->updateMetadata();

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

    m_screen = &screen;

    auto root = m_renderer->getRoot();

    /* ================= STATUS THREAD ================= */

    std::thread status_thread(
      [&]
      {
        while (m_isRunning)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(50));

          if (m_screen)
            m_screen->PostEvent(ftxui::Event::Custom);
        }
      });

    /* ================= MPRIS THREAD ================= */

    std::thread mpris_thread(
      [&]
      {
        while (m_isRunning)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));

          m_mprisService->poll();
        }
      });

    /* ================= SEEK THREAD ================= */

    std::thread seek_thread(
      [&]
      {
        while (m_isRunning)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
      });

    /* ================= MAIN UI LOOP ================= */

    screen.Loop(root);

    /* ================= CLEANUP ================= */

    m_isRunning = false;

    if (status_thread.joinable())
      status_thread.join();

    if (mpris_thread.joinable())
      mpris_thread.join();

    if (seek_thread.joinable())
      seek_thread.join();

    helpers::telemetry::endPlayback(audio, m_telemetryCtx, m_currentPlay, m_lastPlayTick);
    ;
  }

private:
  config::Watcher            m_cfgWatcher;
  threads::SafeMap<SongMap>* m_songMapTS{nullptr};
  telemetry::Context*        m_telemetryCtx{nullptr};
  mpris::Service*            m_mprisService{nullptr};

  std::unique_ptr<TuiRenderer> m_renderer;

  std::atomic<bool>  m_isRunning{false};
  std::optional<i64> m_lastPlayTick;
  // telemetry
  std::optional<telemetry::Event> m_currentPlay; // active play session

  ftxui::ScreenInteractive* m_screen{nullptr};
};

} // namespace frontend::tui
