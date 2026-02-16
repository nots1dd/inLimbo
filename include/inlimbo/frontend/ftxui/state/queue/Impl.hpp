#pragma once

#include <cstddef>

namespace frontend::tui::state::queue
{

class QueueState
{
public:
  void moveSelection(int delta, size_t playlistSize);
  void clamp(size_t playlistSize);

  [[nodiscard]] auto selected() const -> int { return m_selected; }

  void onItemRemoved(size_t playlistSize);

private:
  int m_selected{0};
};

} // namespace frontend::tui::state::queue
