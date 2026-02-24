#include "frontend/ftxui/state/help/Impl.hpp"
#include "utils/Index.hpp"

using namespace ftxui;

namespace frontend::tui::state::help
{

static auto row(std::string_view key, std::string_view desc) -> Element
{
  return hbox({
    text(key.data()) | bold | color(Color::Cyan),
    text("  "),
    text(desc.data()) | color(Color::GrayLight),
  });
}

void HelpState::rebuild(const TuiConfig& cfg)
{
  m_elements.clear();

  const auto kb = cfg.kb;

  m_elements.push_back(vbox({
                         text("██╗███╗   ██╗██╗     ██╗███╗   ███╗██████╗  ██████╗") | bold,
                         text("██║████╗  ██║██║     ██║████╗ ████║██╔══██╗██╔═══██╗"),
                         text("██║██╔██╗ ██║██║     ██║██╔████╔██║██████╔╝██║   ██║"),
                         text("██║██║╚██╗██║██║     ██║██║╚██╔╝██║██╔══██╗██║   ██║"),
                         text("██║██║ ╚████║███████╗██║██║ ╚═╝ ██║██████╔╝╚██████╔╝") | bold,
                         text("╚═╝╚═╝  ╚═══╝╚══════╝╚═╝╚═╝     ╚═╝╚═════╝  ╚═════╝"),
                       }) |
                       center);

  m_elements.push_back(separator());

  auto L_addRow = [&](ftxui::Element e)
  {
    m_selectable.push_back((int)m_elements.size());
    m_elements.push_back(std::move(e));
  };

  // --------------------------------------------------
  // Navigation
  // --------------------------------------------------
  m_elements.push_back(text("Navigation") | bold | color(Color::Yellow));
  L_addRow(row("Tab", "Toggle artist / album focus"));
  L_addRow(row("↑ / ↓", "Move selection"));
  L_addRow(row("g / G", "Jump to top / bottom"));
  L_addRow(row("Enter", "Play selected track"));
  L_addRow(row(std::string(1, kb.gotoLibraryScreen()), "Go to Library"));
  L_addRow(row(std::string(1, kb.gotoLyricsScreen()), "Go to Now Playing"));
  L_addRow(row(std::string(1, kb.gotoQueueScreen()), "Go to Queue"));
  L_addRow(row(std::string(1, kb.gotoHelpScreen()), "Open Help"));

  m_elements.push_back(separator());

  // --------------------------------------------------
  // Playback
  // --------------------------------------------------
  m_elements.push_back(text("Playback") | bold | color(Color::Yellow));
  L_addRow(row(std::string(1, kb.playPause()), "Play / Pause"));
  L_addRow(row(std::string(1, kb.nextTrack()), "Next track"));
  L_addRow(row(std::string(1, kb.prevTrack()), "Previous track"));
  L_addRow(row(std::string(1, kb.randomTrack()), "Random track"));
  L_addRow(row(std::string(1, kb.restartTrack()), "Restart current track"));

  m_elements.push_back(separator());

  // --------------------------------------------------
  // Seeking & Volume
  // --------------------------------------------------
  m_elements.push_back(text("Seeking & Volume") | bold | color(Color::Yellow));
  L_addRow(row(std::string(1, kb.seekBack()), "Seek backward"));
  L_addRow(row(std::string(1, kb.seekFwd()), "Seek forward"));
  L_addRow(row(std::string(1, kb.volUp()), "Volume up"));
  L_addRow(row(std::string(1, kb.volDown()), "Volume down"));
  L_addRow(row(std::string(1, kb.toggleMute()), "Mute / unmute"));

  m_elements.push_back(separator());

  // --------------------------------------------------
  // Queue & Library
  // --------------------------------------------------
  m_elements.push_back(text("Library & Queue") | bold | color(Color::Yellow));
  L_addRow(row("a", "Add track / artist to queue"));
  L_addRow(row("d", "Remove track from queue"));
  L_addRow(row("l", "Fetch lyrics (Now Playing)"));

  m_elements.push_back(separator());

  // --------------------------------------------------
  // Global
  // --------------------------------------------------
  m_elements.push_back(text("Global") | bold | color(Color::Yellow));
  L_addRow(row(std::string(1, kb.quit()), "Quit application"));
  L_addRow(row("Esc", "Back / close screen"));

  m_elements.push_back(separator());
  m_elements.push_back(text("Press your configured Library key or Esc to return") | dim | center);

  m_selected = 0;
}

void HelpState::moveSelection(int delta)
{
  if (m_selectable.empty())
    return;

  auto it = std::ranges::find(m_selectable, m_selected);
  if (it == m_selectable.end())
  {
    m_selected = m_selectable.front();
    return;
  }

  const auto idx   = std::distance(m_selectable.begin(), it);
  const auto count = m_selectable.size();

  std::optional<size_t> next;

  if (delta > 0)
    next = utils::index::nextWrap(idx, count);
  else if (delta < 0)
    next = utils::index::prevWrap(idx, count);
  else
    return;

  if (next)
    m_selected = m_selectable[*next];
}

} // namespace frontend::tui::state::help
