#include "frontend/ftxui/ui/screens/Main.hpp"
#include "frontend/ftxui/state/library/Impl.hpp"

using namespace ftxui;

namespace frontend::tui::ui::screens
{

MainScreen::MainScreen(state::library::LibraryState& state) : m_state(state)
{
  artist_menu = Menu(&m_state.artists, &m_state.selected_artist) | vscroll_indicator;

  album_content =
    Renderer([&]() -> Element { return vbox(m_state.returnAlbumElements()) | vscroll_indicator; });

  album_scroll        = 0.0f;
  album_scroll_target = 0.0f;

  album_view = Renderer(album_content,
                        [&]() -> Element
                        {
                          const auto& elems = m_state.returnAlbumElements();

                          if (!elems.empty())
                          {
                            album_scroll_target = float(m_state.selected_album_index) /
                                                  float(std::max<int>(1, elems.size() - 1));
                          }

                          constexpr float smoothing = 0.15f; // 0.1 = slow, 0.25 = fast
                          album_scroll += (album_scroll_target - album_scroll) * smoothing;

                          album_scroll = std::clamp(album_scroll, 0.0f, 1.0f);

                          return album_content->Render() |
                                 focusPositionRelative(0.0f, album_scroll) | frame | flex;
                        });

  container = Container::Horizontal({
    artist_menu,
    album_view,
  });
}

void MainScreen::syncFocus()
{
  if (m_state.focusOnArtists())
    artist_menu->TakeFocus();
  else
    album_view->TakeFocus();
}

auto MainScreen::component() -> Component { return container; }

auto MainScreen::render() -> Element
{
  syncFocus();

  if (m_state.selected_artist != m_state.current_artist_cache)
  {
    m_state.rebuildForSelectedArtist(m_state.selected_artist);
  }

  if (m_audioPtr)
  {
    m_state.decorateAlbumViewSelection(m_state.selected_album_index,
                                       m_audioPtr->getCurrentMetadata());
  }

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  auto artist_inner = window(text(" Artists ") | bold, artist_menu->Render() | frame | flex) |
                      size(WIDTH, EQUAL, half_width);

  auto album_inner = window(text(" Songs ") | bold, album_view->Render() | frame | flex) |
                     size(WIDTH, EQUAL, term.dimx - half_width);

  Color artist_border_color = m_state.focusOnArtists() ? Color::Green : Color::GrayDark;

  Color album_border_color = !m_state.focusOnArtists() ? Color::Green : Color::GrayDark;

  auto artist_pane = artist_inner | borderStyled(BorderStyle::HEAVY, artist_border_color);

  auto album_pane = album_inner | borderStyled(BorderStyle::HEAVY, album_border_color);

  return vbox({hbox({artist_pane, album_pane})});
}

} // namespace frontend::tui::ui::screens
