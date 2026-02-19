#include "frontend/ftxui/state/now_playing/Impl.hpp"
#include "frontend/ftxui/state/now_playing/LRC.hpp"
#include "helpers/fs/LRC.hpp"
#include "lrc/Client.hpp"

namespace frontend::tui::state::now_playing
{

auto NowPlayingState::lyricsFetchState() const -> LyricsFetchState { return m_fetch_state; }

auto NowPlayingState::lyricsError() const -> const std::string& { return m_fetch_error; }

auto NowPlayingState::hasLyrics() const -> bool
{
  return !m_lyrics.empty() &&
         !(m_lyrics.size() == 1 && m_lyrics[0].find("Lyrics not found") != std::string::npos);
}

void NowPlayingState::fetchLyricsFromLRCAsync(const Metadata& meta, int wrap_width,
                                              managers::ThreadManager& tm)
{
  if (m_fetch_state == LyricsFetchState::Fetching)
    return;

  m_fetch_state = LyricsFetchState::Fetching;
  m_fetch_error.clear();

  tm.execute(
    [this, meta, wrap_width]() -> void
    {
      auto cachePath = helpers::lrc::genLRCFilePath(meta.artist, meta.title, meta.album);

      if (helpers::lrc::tryReadCachedLRC(cachePath))
      {
        m_fetch_state = LyricsFetchState::Ready;
        loadLyrics(meta, wrap_width);
        return;
      }

      ::lrc::Client client;

      ::lrc::Query query;
      query.artist = meta.artist;
      query.album  = meta.album;
      query.track  = meta.title;

      auto res = client.fetchBestMatchAndCache(query);

      if (!res.ok())
      {
        m_fetch_error = res.error.message;
        m_fetch_state = LyricsFetchState::Error;
        loadLyrics(meta, wrap_width, false);
        return;
      }

      loadLyrics(meta, wrap_width);
      m_fetch_state = LyricsFetchState::Ready;
    });
}

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

auto NowPlayingState::renderLyrics() -> std::vector<ftxui::Element>
{
  using namespace ftxui;
  std::vector<Element> out;

  for (int i = 0; i < (int)m_lyrics.size(); ++i)
  {
    Element line = text(m_lyrics[i]);

    if (i == m_selected_index)
      line = line | bgcolor(Color::RGB(40, 60, 90)) | color(Color::White) | bold;

    if (m_lyrics[i].empty())
      line = line | dim;

    out.push_back(line);
  }

  if (out.empty())
    out.push_back(text("No lyrics available.") | dim | center);

  return out;
}

void NowPlayingState::loadLyrics(const Metadata& meta, int wrap_width, const bool refreshFetchState)
{
  if (refreshFetchState)
  {
    m_fetch_state = LyricsFetchState::Idle;
    m_fetch_error.clear();
  }

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
      raw_lyrics = *cached;
      m_source   = LyricsSource::LrcCache;
      m_source_info =
        "Source: LRC cache (" + utils::string::transform::trim(cachePath.string(), 30) + ")";
    }
  }

  if (raw_lyrics.empty())
  {
    m_lyrics = {
      "Lyrics not found.", "", "Attempted sources:", " • Embedded metadata", " • Local LRC cache",
    };

    if (m_fetch_state == LyricsFetchState::Error && !m_fetch_error.empty())
    {
      m_lyrics.emplace_back("");
      m_lyrics.emplace_back("Last fetch error:");
      m_lyrics.emplace_back(" • " + m_fetch_error);
    }

    m_lyrics.insert(m_lyrics.end(),
                    {"", "Artist: " + meta.artist, "Title : " + meta.title, "Album : " + meta.album,
                     "", "Press 'l' to fetch lyrics (requires internet)", "",
                     "NOTE: Some songs may not have LRC files - fetching may fail."});

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
