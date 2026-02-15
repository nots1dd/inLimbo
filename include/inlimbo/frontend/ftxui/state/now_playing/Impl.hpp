#pragma once

#include "InLimbo-Types.hpp"
#include "utils/ClassRulesMacros.hpp"
#include <string>
#include <vector>

namespace frontend::tui::state::now_playing
{

enum class LyricsSource
{
  Metadata,
  LrcCache,
  None
};

class NowPlayingState
{
public:
  DEFAULT_CTOR(NowPlayingState);

  void               loadLyrics(const Metadata& meta, int wrap_width);
  auto               lyrics() -> std::vector<std::string>&;
  auto               selectedIndex() -> int&;
  auto               setSelectedIndex(int index) -> void;
  [[nodiscard]] auto sourceInfo() const -> const std::string&;

private:
  std::vector<std::string> m_lyrics;
  int                      m_selected_index{0};
  LyricsSource             m_source{LyricsSource::None};
  std::string              m_source_info;
};

} // namespace frontend::tui::state::now_playing
