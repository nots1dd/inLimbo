#include "frontend/ftxui/state/queue/Impl.hpp"
#include "utils/Index.hpp"

namespace frontend::tui::state::queue
{

void QueueState::clamp(size_t playlistSize)
{
  if (playlistSize == 0)
  {
    m_selected = 0;
    return;
  }

  if (m_selected < 0)
    m_selected = 0;
  else if ((size_t)m_selected >= playlistSize)
    m_selected = static_cast<int>(playlistSize - 1);
}

void QueueState::moveSelection(int delta, size_t playlistSize)
{
  if (playlistSize == 0)
    return;

  const auto current = static_cast<size_t>(m_selected);

  std::optional<size_t> next;
  if (delta > 0)
    next = utils::index::nextWrap(current, playlistSize);
  else if (delta < 0)
    next = utils::index::prevWrap(current, playlistSize);
  else
    return;

  if (next)
    m_selected = static_cast<int>(*next);
}

// to be called after any change to the playlist, to ensure the selected index is still valid
void QueueState::onItemRemoved(size_t playlistSize) { clamp(playlistSize); }

} // namespace frontend::tui::state::queue
