#pragma once

#include "audio/Service.hpp"
#include "frontend/ftxui/state/album_art/Impl.hpp"
#include "frontend/ftxui/state/now_playing/Impl.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::tui::ui::screens
{

class NowPlayingScreen
{
public:
  NowPlayingScreen(state::now_playing::NowPlayingState& nowState,
                   state::album_art::AlbumArtState&     artState);

  auto render() -> ftxui::Element;

  void attachAudioService(audio::Service& audio) { m_audioPtr = &audio; }

private:
  state::now_playing::NowPlayingState& m_now;
  state::album_art::AlbumArtState&     m_art;
  audio::Service*                      m_audioPtr;

  ftxui::Component lyrics_content;
  ftxui::Component lyrics_view;
};

} // namespace frontend::tui::ui::screens
