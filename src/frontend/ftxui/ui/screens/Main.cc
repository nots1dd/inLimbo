#include "frontend/ftxui/ui/screens/Main.hpp"
#include "frontend/ftxui/components/scroll/Scrollable.hpp"
#include "frontend/ftxui/state/library/Impl.hpp"

using namespace ftxui;

namespace frontend::tui::ui::screens
{

MainScreen::MainScreen(state::library::LibraryState& state) : m_state(state)
{
  artist_menu = Menu(&m_state.artists, &m_state.selected_artist) | vscroll_indicator;

  album_content =
    Renderer([&]() mutable -> Element { return vbox(m_state.returnAlbumElements()); });

  album_scroller =
    Scroller(album_content, &m_state.selected_album_index, Color::Green, Color::GrayDark);

  container = Container::Horizontal({artist_menu, album_scroller});
}

void MainScreen::syncFocus()
{
  if (m_state.focusOnArtists())
    artist_menu->TakeFocus();
  else
    album_scroller->TakeFocus();
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

  auto album_inner = window(text(" Songs ") | bold, album_scroller->Render() | frame | flex) |
                     size(WIDTH, EQUAL, term.dimx - half_width);

  Color artist_border_color = m_state.focusOnArtists() ? Color::Green : Color::GrayDark;

  Color album_border_color = !m_state.focusOnArtists() ? Color::Green : Color::GrayDark;

  auto artist_pane = artist_inner | borderStyled(BorderStyle::HEAVY, artist_border_color);

  auto album_pane = album_inner | borderStyled(BorderStyle::HEAVY, album_border_color);

  return vbox({hbox({artist_pane, album_pane})});
}

} // namespace frontend::tui::ui::screens
