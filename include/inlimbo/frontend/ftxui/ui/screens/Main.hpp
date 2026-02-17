#pragma once

#include "audio/Service.hpp"
#include "frontend/ftxui/state/library/Impl.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::tui::ui::screens
{

class MainScreen
{
public:
  MainScreen(state::library::LibraryState& state);

  auto component() -> ftxui::Component;
  auto render() -> ftxui::Element;

  void attachAudioService(audio::Service& audio) { m_audioPtr = &audio; }

private:
  state::library::LibraryState& m_state;
  audio::Service*               m_audioPtr;

  ftxui::Component artist_menu;
  ftxui::Component album_content;
  ftxui::Component album_scroller;
  ftxui::Component container;
  ftxui::Component root_renderer;

  bool focus_on_artists{true};

  void syncFocus();
};

} // namespace frontend::tui::ui::screens
