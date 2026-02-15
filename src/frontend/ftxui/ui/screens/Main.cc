#include "frontend/ftxui/ui/screens/Main.hpp"
#include "frontend/ftxui/components/scroll/Scrollable.hpp"
#include "frontend/ftxui/state/library/Impl.hpp"
#include "utils/timer/Timer.hpp"

using namespace ftxui;

namespace frontend::tui::ui::screens
{

MainScreen::MainScreen(state::library::LibraryState& state) : m_state(state)
{
  artist_menu = Menu(&m_state.artists, &m_state.selected_artist);

  album_content = Renderer(
    [&]() mutable -> Element
    {
      Elements rows;

      for (size_t i = 0; i < m_state.album_view_lines.size(); ++i)
      {
        const auto& line    = m_state.album_view_lines[i];
        auto        songObj = m_state.view_song_objects[i];

        bool    selected = (int)i == m_state.selected_album_index;
        Element selector = selected ? text("➤ ") | color(Color::Yellow) : text("  ");

        if (songObj)
        {
          std::string dur = utils::timer::fmtTime(songObj->metadata.duration);

          auto row = hbox({text("  "), text(line), filler(), text(dur) | dim});

          if (selected)
          {
            row = row | bgcolor(Color::RGB(40, 60, 90)) | color(Color::White) | bold;
          }

          if (m_audioPtr->isPlaying())
          {
            auto meta = m_audioPtr->getCurrentMetadata();
            if (meta && meta->title == songObj->metadata.title)
              row = row | color(Color::Cyan);
          }

          rows.push_back(row);
        }
        else
        {
          bool is_disc = line.starts_with("Disc ");

          Element header = hbox({selector, text(" " + line)});

          if (is_disc)
            header = header | bgcolor(Color::RGB(35, 35, 35)) | color(Color::YellowLight) | bold;
          else
            header = header | bgcolor(Color::RGB(25, 25, 25)) | color(Color::Cyan) | bold;

          rows.push_back(header);
        }
      }

      return vbox(rows) | frame;
    });

  album_scroller =
    Scroller(album_content, &m_state.selected_album_index, Color::Green, Color::GrayDark) | frame | flex;

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

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  auto artist_inner = window(text(" Artists ") | bold, artist_menu->Render() | frame | flex) |
                      size(WIDTH, EQUAL, half_width);

  auto album_inner = window(text(" Albums ") | bold, album_scroller->Render() | frame | flex) |
                     size(WIDTH, EQUAL, term.dimx - half_width);

  Color artist_border_color = m_state.focusOnArtists() ? Color::Green : Color::GrayDark;

  Color album_border_color = !m_state.focusOnArtists() ? Color::Green : Color::GrayDark;

  auto artist_pane = artist_inner | borderStyled(BorderStyle::HEAVY, artist_border_color);

  auto album_pane = album_inner | borderStyled(BorderStyle::HEAVY, album_border_color);

  return hbox({artist_pane, album_pane}) | flex;
}

} // namespace frontend::tui::ui::screens
