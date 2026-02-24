#pragma once

#include "audio/Service.hpp"
#include "frontend/ftxui/managers/Threads.hpp"
#include "frontend/ftxui/state/help/Impl.hpp"
#include "frontend/ftxui/state/library/Impl.hpp"
#include "frontend/ftxui/state/now_playing/Impl.hpp"
#include "frontend/ftxui/state/queue/Impl.hpp"
#include "mpris/Service.hpp"
#include <ftxui/component/component.hpp>

namespace frontend::tui
{
enum class UIScreen
{
  Main,
  NowPlaying,
  Help,
  Config,
  Queue
};
}

namespace frontend::tui::ui
{

class EventHandler
{
public:
  EventHandler(UIScreen& activeScreen, state::library::LibraryState& libraryState,
               state::help::HelpState& helpState, state::now_playing::NowPlayingState& nowState,
               state::queue::QueueState& queueState, managers::ThreadManager& threadManager,
               TS_SongMap* songMap, mpris::Service* mpris);

  auto handle(ftxui::Event e) -> bool;

  void attachAudioService(audio::Service& audio) { m_audioPtr = &audio; }

private:
  UIScreen& m_activeScreen;

  state::library::LibraryState&        m_libraryState;
  state::help::HelpState&              m_helpState;
  state::now_playing::NowPlayingState& m_nowState;
  state::queue::QueueState&            m_queueState;

  managers::ThreadManager& m_threadManager;

  float m_volume{0.0f};

  audio::Service* m_audioPtr;
  TS_SongMap*     m_songMap{nullptr};
  mpris::Service* m_mpris{nullptr};

  void playSelected();
  void adjustVolume(float delta);
  void toggleMute();
};

} // namespace frontend::tui::ui
