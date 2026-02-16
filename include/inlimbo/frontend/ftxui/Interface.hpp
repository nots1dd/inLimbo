#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "audio/Service.hpp"
#include "frontend/ftxui/managers/Threads.hpp"
#include "frontend/ftxui/state/album_art/Impl.hpp"
#include "frontend/ftxui/state/now_playing/Impl.hpp"
#include "frontend/ftxui/ui/screens/Main.hpp"
#include "frontend/ftxui/ui/screens/Queue.hpp"
#include "mpris/Service.hpp"
#include "thread/Map.hpp"

#include "state/album_art/Impl.hpp"
#include "state/library/Impl.hpp"
#include "state/now_playing/Impl.hpp"

#include "ui/EventHandler.hpp"
#include "ui/screens/Main.hpp"
#include "ui/screens/NowPlaying.hpp"

namespace frontend::tui
{

enum class StatusBarMode
{
  Full,
  Reduced,
  Compact,
};

inline static auto statusBarMode() -> StatusBarMode
{
  const int w = ftxui::Terminal::Size().dimx;

  if (w >= 120)
    return StatusBarMode::Full;
  if (w >= 90)
    return StatusBarMode::Reduced;
  return StatusBarMode::Compact;
}

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>* songMap, telemetry::Context* telemetry,
                     mpris::Service* mpris);

  void run(audio::Service& audio);

  IMMUTABLE(Interface);

  auto getRoot() -> ftxui::Component;

private:
  auto renderRoot() -> ftxui::Element;
  auto renderTitleBar() -> ftxui::Element;
  auto renderStatusBar() -> ftxui::Element;

  auto renderStatusBarFull() -> ftxui::Element;
  auto renderStatusBarReduced() -> ftxui::Element;
  auto renderStatusBarCompact() -> ftxui::Element;

  void playSelected();

  threads::SafeMap<SongMap>*                            m_songMapTS{nullptr};
  telemetry::Context*                                   m_telemetryCtx{nullptr};
  mpris::Service*                                       m_mpris{nullptr};
  std::optional<std::reference_wrapper<audio::Service>> m_audio;

  ftxui::ScreenInteractive* m_screen{nullptr};

  managers::ThreadManager m_threadManager;

  UIScreen          active_screen{UIScreen::Main};
  std::atomic<bool> m_needsRebuild{false};

  state::library::LibraryState        m_libraryState;
  state::now_playing::NowPlayingState m_nowState;
  state::album_art::AlbumArtState     m_albumArtState;
  state::queue::QueueState            m_queueState;

  ui::screens::MainScreen       m_mainScreen;
  ui::screens::NowPlayingScreen m_nowPlayingScreen;
  ui::screens::QueueScreen      m_queueScreen;

  ui::EventHandler m_eventHandler;
  ftxui::Component renderer;
};

} // namespace frontend::tui
