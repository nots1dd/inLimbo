#include "frontend/ftxui/ui/screens/Main.hpp"
#include "frontend/ftxui/state/library/Impl.hpp"

using namespace ftxui;

namespace frontend::tui::ui::screens
{

MainScreen::MainScreen(state::library::LibraryState& state) : m_state(state)
{
  artist_content = Renderer(
    [&]() -> Element
    {
      Elements rows;

      const bool focused = m_state.focusOnArtists();

      for (int i = 0; i < (int)m_state.artists.size(); ++i)
      {
        Element row = text(" " + m_state.artists[i]);

        if (i == m_state.selected_artist)
        {
          if (focused)
          {
            row = row | bgcolor(Color::RGB(40, 60, 90)) | color(Color::White) | bold;
          }
          else
          {
            row = row | bgcolor(Color::GrayDark) | color(Color::RGB(180, 180, 180)) | bold;
          }
        }
        else
        {
          row = row | dim;
        }

        rows.push_back(row);
      }

      if (rows.empty())
        rows.push_back(text("No artists") | dim | center);

      return vbox(rows);
    });

  artist_scroll = 0.0f;

  artist_view = Renderer(artist_content,
                         [&]() -> Element
                         {
                           const int count = (int)m_state.artists.size();
                           if (count > 1)
                             artist_scroll = float(m_state.selected_artist) / float(count - 1);
                           else
                             artist_scroll = 0.0f;

                           return artist_content->Render() |
                                  focusPositionRelative(0.0f, artist_scroll) | frame | flex;
                         });

  album_content =
    Renderer([&]() -> Element { return vbox(m_state.returnAlbumElements()) | vscroll_indicator; });

  album_scroll = 0.0f;

  album_view = Renderer(album_content,
                        [&]() -> Element
                        {
                          const auto& elems = m_state.returnAlbumElements();

                          if (!elems.empty())
                          {
                            album_scroll = float(m_state.selected_album_index) /
                                           float(std::max<int>(1, elems.size() - 1));
                          }

                          return album_content->Render() |
                                 focusPositionRelative(0.0f, album_scroll) | frame | flex;
                        });

  container = Container::Horizontal({
    artist_view,
    album_view,
  });
}

void MainScreen::syncFocus()
{
  if (m_state.focusOnArtists())
    artist_view->TakeFocus();
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
                                       m_audioPtr->getCurrentMetadata(), !m_state.focusOnArtists());
  }

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  auto artist_inner = window(text(" Artists ") | bold, artist_view->Render() | frame | flex) |
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
