#pragma once

#include "InLimbo-Types.hpp"
#include "frontend/ftxui/managers/Threads.hpp"
#include "utils/ClassRulesMacros.hpp"
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

namespace frontend::tui::state::now_playing
{

enum class LyricsFetchState
{
  Idle,
  Fetching,
  Ready,
  Error
};

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

  auto               renderLyrics() -> std::vector<ftxui::Element>;
  void               loadLyrics(const Metadata& meta, int wrap_width);
  auto               lyrics() -> std::vector<std::string>&;
  auto               selectedIndex() -> int&;
  auto               setSelectedIndex(int index) -> void;
  [[nodiscard]] auto sourceInfo() const -> const std::string&;
  void fetchLyricsFromLRCAsync(const Metadata& meta, int wrap_width, managers::ThreadManager& tm);
  [[nodiscard]] auto hasLyrics() const -> bool;

  [[nodiscard]] auto lyricsFetchState() const -> LyricsFetchState;
  [[nodiscard]] auto lyricsError() const -> const std::string&;

private:
  std::vector<std::string> m_lyrics;
  int                      m_selected_index{0};
  LyricsSource             m_source{LyricsSource::None};
  std::string              m_source_info;
  LyricsFetchState         m_fetch_state = LyricsFetchState::Idle;
  std::string              m_fetch_error;
};

} // namespace frontend::tui::state::now_playing
