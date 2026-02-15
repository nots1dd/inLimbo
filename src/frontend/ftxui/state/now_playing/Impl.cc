#include "frontend/ftxui/state/now_playing/Impl.hpp"
#include "frontend/ftxui/state/now_playing/LRC.hpp"
#include "helpers/fs/LRC.hpp"

namespace frontend::tui::state::now_playing
{

auto NowPlayingState::lyrics() -> std::vector<std::string>& { return m_lyrics; }

auto NowPlayingState::selectedIndex() -> int& { return m_selected_index; }

auto NowPlayingState::setSelectedIndex(int index) -> void
{
  if (m_lyrics.empty())
  {
    m_selected_index = 0;
    return;
  }

  int count = static_cast<int>(m_lyrics.size());

  if (index < 0)
    m_selected_index = count - 1;
  else if (index >= count)
    m_selected_index = 0;
  else
    m_selected_index = index;
}

auto NowPlayingState::sourceInfo() const -> const std::string& { return m_source_info; }

void NowPlayingState::loadLyrics(const Metadata& meta, int wrap_width)
{
  m_lyrics.clear();
  m_selected_index = 0;
  m_source         = LyricsSource::None;
  m_source_info.clear();

  std::string raw_lyrics;

  if (!meta.lyrics.empty())
  {
    raw_lyrics    = meta.lyrics;
    m_source      = LyricsSource::Metadata;
    m_source_info = "Source: Embedded metadata";
  }
  else
  {
    auto cachePath = helpers::lrc::genLRCFilePath(meta.artist, meta.title, meta.album);

    if (auto cached = helpers::lrc::tryReadCachedLRC(cachePath))
    {
      raw_lyrics    = *cached;
      m_source      = LyricsSource::LrcCache;
      m_source_info = "Source: LRC cache (" + cachePath.string() + ")";
    }
  }

  if (raw_lyrics.empty())
  {
    m_lyrics = {"Lyrics not found.",    "", "Attempted sources:",     " • Embedded metadata",
                " • Local LRC cache",   "", "Artist: " + meta.artist, "Title : " + meta.title,
                "Album : " + meta.album};
    return;
  }

  std::stringstream ss(raw_lyrics);
  std::string       line;

  while (std::getline(ss, line))
  {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    line = detail::lrcStripTags(line);

    if (line.empty())
    {
      m_lyrics.emplace_back("");
      continue;
    }

    size_t open  = line.find('[');
    size_t close = line.find(']', open);

    if (open != std::string::npos && close != std::string::npos)
    {
      std::string token = line.substr(open, close - open + 1);

      if (detail::lrcIsSectionHeader(token))
      {
        m_lyrics.emplace_back("");
        m_lyrics.push_back(token);
        m_lyrics.emplace_back("");

        line.erase(open, close - open + 1);

        if (line.empty())
          continue;
      }
    }

    std::istringstream words(line);
    std::string        word;
    std::string        current_line;

    while (words >> word)
    {
      if ((int)(current_line.size() + word.size() + 1) > wrap_width)
      {
        if (!current_line.empty())
          m_lyrics.push_back(current_line);
        current_line = word;
      }
      else
      {
        if (!current_line.empty())
          current_line += " ";
        current_line += word;
      }
    }

    if (!current_line.empty())
      m_lyrics.push_back(current_line);
  }

  if (m_lyrics.empty())
    m_lyrics.emplace_back("Lyrics loaded but empty.");

  m_lyrics.insert(m_lyrics.begin(), "");
  m_lyrics.insert(m_lyrics.begin(), m_source_info);
}

} // namespace frontend::tui::state::now_playing
