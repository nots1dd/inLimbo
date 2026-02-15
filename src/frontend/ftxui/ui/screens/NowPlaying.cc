#include "frontend/ftxui/ui/screens/NowPlaying.hpp"
#include "frontend/ftxui/components/scroll/Scrollable.hpp"
#include "utils/fs/FileUri.hpp"

using namespace ftxui;

namespace frontend::tui::ui::screens
{

NowPlayingScreen::NowPlayingScreen(state::now_playing::NowPlayingState& nowState,
                                   state::album_art::AlbumArtState&     artState)
    : m_now(nowState), m_art(artState)
{
  lyrics_menu = Menu(&m_now.lyrics(), &m_now.selectedIndex());

  lyrics_scroller = Scroller(lyrics_menu, &m_now.selectedIndex(), Color::Cyan, Color::GrayDark);
}

auto NowPlayingScreen::render() -> Element
{
  if (!m_audioPtr)
    return text("No Track");

  auto meta      = m_audioPtr->getCurrentMetadata();
  auto trackInfo = m_audioPtr->getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("No Track");

  static PathStr last_art_path;

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  int wrap_width = std::max(10, term.dimx - half_width - 6);

  static int last_wrap_width = 0;

  if (wrap_width != last_wrap_width)
  {
    last_wrap_width = wrap_width;
    m_now.loadLyrics(*meta, wrap_width);
  }

  if (meta->artUrl != last_art_path)
  {
    last_art_path = meta->artUrl;

    m_art.load(utils::fs::fromAbsFilePathUri(meta->artUrl.c_str()).c_str());
    m_now.loadLyrics(*meta, wrap_width);
  }

  int content_height = term.dimy - 4;

  auto left =
    m_art.render() | size(WIDTH, EQUAL, half_width) | size(HEIGHT, EQUAL, content_height) | border;

  auto lyrics_view = window(text(" Lyrics ") | bold, lyrics_scroller->Render() | frame | flex) |
                     size(WIDTH, EQUAL, term.dimx - half_width) |
                     size(HEIGHT, EQUAL, content_height) | border;

  return hbox({left, lyrics_view}) | flex | bgcolor(Color::RGB(18, 18, 18));
}

} // namespace frontend::tui::ui::screens
