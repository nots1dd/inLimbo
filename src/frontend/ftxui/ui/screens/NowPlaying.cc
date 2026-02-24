#include "frontend/ftxui/ui/screens/NowPlaying.hpp"
#include "utils/fs/FileUri.hpp"

using namespace ftxui;

namespace frontend::tui::ui::screens
{

NowPlayingScreen::NowPlayingScreen(state::now_playing::NowPlayingState& nowState,
                                   state::album_art::AlbumArtState&     artState)
    : m_now(nowState), m_art(artState)
{
  lyrics_content = Renderer([&]() -> Element { return vbox(m_now.renderLyrics()); });

  lyrics_view = Renderer(
    lyrics_content,
    [&]() -> Element
    {
      const auto& lines    = m_now.lyrics();
      float       scroll_y = 0.0f;

      if (!lines.empty())
      {
        scroll_y = float(m_now.selectedIndex()) / float(std::max<int>(1, int(lines.size()) - 1));
      }

      return lyrics_content->Render() | focusPositionRelative(0.0f, scroll_y) | frame | flex;
    });
}

auto NowPlayingScreen::render() -> Element
{
  if (!m_audioPtr)
    return text("No Track");

  auto meta      = m_audioPtr->getCurrentMetadata();
  auto trackInfo = m_audioPtr->getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("No Track");

  static PathStr last_art_url;
  static int     last_wrap_width = 0;

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  int wrap_width = std::max(10, term.dimx - half_width - 6);

  if (wrap_width != last_wrap_width)
  {
    last_wrap_width = wrap_width;
    m_now.loadLyrics(*meta, wrap_width);
  }

  if (meta->artUrl != last_art_url)
  {
    last_art_url = meta->artUrl;
    m_art.load(utils::fs::fromAbsFilePathUri(meta->artUrl).c_str());
    m_now.loadLyrics(*meta, wrap_width);
  }

  int content_height = term.dimy - 4;

  auto left =
    m_art.render() | size(WIDTH, EQUAL, half_width) | size(HEIGHT, EQUAL, content_height) | border;

  Element lyrics_header = hbox({text(" Lyrics ") | bold, filler(), [&]() -> Element
                                {
                                  using namespace state::now_playing;

                                  switch (m_now.lyricsFetchState())
                                  {
                                    case LyricsFetchState::Fetching:
                                      return text("Fetching <-.-> ") | color(Color::Yellow);

                                    case LyricsFetchState::Error:
                                      return text("Error <!.!> ") | color(Color::Red);

                                    case LyricsFetchState::Ready:
                                      return text("<:3> ") | dim;

                                    case LyricsFetchState::Idle:
                                    default:
                                      return text("<=.=> ") | color(Color::Cyan);
                                  }
                                }()});

  auto right = window(lyrics_header, lyrics_view->Render()) |
               size(WIDTH, EQUAL, term.dimx - half_width) | size(HEIGHT, EQUAL, content_height) |
               borderStyled(BorderStyle::HEAVY, Color::Green);

  return hbox({left, right}) | flex | bgcolor(Color::RGB(18, 18, 18));
}

} // namespace frontend::tui::ui::screens
