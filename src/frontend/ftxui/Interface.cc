#include "frontend/ftxui/Interface.hpp"
#include "helpers/fs/LRC.hpp"
#include "utils/fs/FileUri.hpp"
#include "utils/fs/Paths.hpp"
#include "utils/timer/Timer.hpp"
#include "query/SongMap.hpp"
#include "frontend/ftxui/components/scroll/Scrollable.hpp"
#include "Logger.hpp"
#include "helpers/telemetry/Playback.hpp"
#include <cstring>
#include <thread>
#include <chrono>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace ftxui;

namespace frontend::tui
{

/* ================= AUTO NEXT ================= */

static auto autoNextIfFinished(audio::Service& audio,
                                      mpris::Service& mpris,
                                      std::atomic<ui8>& lastTid,
                                      std::atomic<bool>& inProgress) -> bool
{
  if (inProgress.load(std::memory_order_acquire))
    return false;

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
    return false;

  const auto& info = *infoOpt;

  if (info.lengthSec <= 0.0)
    return false;

  constexpr double EPS = 0.05;
  if (info.positionSec + EPS < info.lengthSec)
    return false;

  if (info.tid == lastTid.load(std::memory_order_relaxed))
    return false;

  bool expected = false;
  if (!inProgress.compare_exchange_strong(expected, true))
    return false;

  lastTid.store(info.tid, std::memory_order_relaxed);

  audio.nextTrackGapless();
  mpris.updateMetadata();
  mpris.notify();

  inProgress.store(false, std::memory_order_release);
  return true;
}

/* ================= CONSTRUCTOR ================= */

Interface::Interface(threads::SafeMap<SongMap>* songMap,
                     telemetry::Context* telemetry,
                     mpris::Service* mprisService)
  : m_cfgWatcher(utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME)),
    m_songMapTS(songMap),
    m_telemetryCtx(telemetry),
    m_mpris(mprisService)
{
  rebuildLibrary();
  current_artist_cache = selected_artist;
  createComponents();
}

auto Interface::getRoot() -> Component
{
  return renderer;
}

/* ================= RUN ================= */

void Interface::run(audio::Service& audio)
{
  query::songmap::read::forEachSongInAlbum(
    *m_songMapTS,
    audio.getCurrentMetadata()->artist,
    audio.getCurrentMetadata()->album,
    [&](const Disc&, const Track&, const ino_t,
        const std::shared_ptr<Song>& song) -> void
    {
      if (song->metadata.track <= audio.getCurrentMetadata()->track)
        return;

      auto h = audio.registerTrack(song);
      audio.addToPlaylist(h);
    });

  m_isRunning = true;
  m_audio     = &audio;

  m_mpris->updateMetadata();

  std::thread status_thread(&Interface::runStatusLoop, this, std::ref(audio));
  std::thread mpris_thread(&Interface::runMprisLoop, this);
  std::thread seek_thread(&Interface::runSeekLoop, this, std::ref(audio));

  ScreenInteractive screen = ScreenInteractive::Fullscreen();
  m_screen = &screen;

  screen.Loop(getRoot());

  m_isRunning = false;

  status_thread.join();
  mpris_thread.join();
  seek_thread.join();
}

/* ================= LIBRARY ================= */

void Interface::rebuildLibrary()
{
  artists.clear();
  album_view_lines.clear();
  view_song_objects.clear();

  if (!m_songMapTS)
    return;

  query::songmap::read::forEachArtist(
    *m_songMapTS,
    [&](const Artist& artist, const AlbumMap&)
    {
      artists.push_back(artist);
    });

  if (!artists.empty())
    buildAlbumViewForArtist(artists[0]);
}

void Interface::rebuildAlbumsForSelectedArtist()
{
  if (selected_artist < 0 || selected_artist >= (int)artists.size())
    return;

  buildAlbumViewForArtist(artists[selected_artist]);
  selected_album_index = 0;
  current_artist_cache = selected_artist;
}

void Interface::buildAlbumViewForArtist(const Artist& artist)
{
  album_view_lines.clear();
  view_song_objects.clear();

  Album current_album;
  Disc  current_disc = -1;

  query::songmap::read::forEachSong(
    *m_songMapTS,
    [&](const Artist& a, const Album& album,
        const Disc disc, const Track track,
        const ino_t, const std::shared_ptr<Song>& song)
    {
      if (a != artist)
        return;

      if (album != current_album)
      {
        current_album = album;
        current_disc  = -1;

        album_view_lines.push_back(album);
        view_song_objects.push_back(nullptr);
      }

      if (disc != current_disc)
      {
        current_disc = disc;
        album_view_lines.push_back("Disc " + std::to_string(disc));
        view_song_objects.push_back(nullptr);
      }

      std::string line =
        (track < 10 ? "0" : "") + std::to_string(track) +
        "  " + song->metadata.title;

      album_view_lines.push_back(line);
      view_song_objects.push_back(song);
    });
}

/* ================= COMPONENT SETUP ================= */

void Interface::createComponents()
{
  artist_menu     = Menu(&artists, &selected_artist);
  lyrics_menu     = Menu(&lyrics_lines, &selected_lyric_index);
  lyrics_scroller = Scroller(lyrics_menu, &selected_lyric_index,
                             Color::Cyan, Color::GrayDark);

  album_content = Renderer(
    [&]() -> Element
    {
      Elements rows;

      for (size_t i = 0; i < album_view_lines.size(); ++i)
      {
        const auto& line    = album_view_lines[i];
        auto        songObj = view_song_objects[i];

        bool selected = (int)i == selected_album_index;
        Element selector = selected
          ? text("➤ ") | color(Color::Yellow)
          : text("  ");

        if (songObj)
        {
          std::string dur =
            utils::timer::fmtTime(songObj->metadata.duration);

          auto row = hbox({
            text("  "),
            text(line),
            filler(),
            text(dur) | dim
          });

          if (selected)
          {
            row = row
                  | bgcolor(Color::RGB(40, 60, 90))
                  | color(Color::White)
                  | bold;
          }

          if (m_audio && m_audio->isPlaying())
          {
            auto meta = m_audio->getCurrentMetadata();
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
            header = header | bgcolor(Color::RGB(35,35,35))
                              | color(Color::YellowLight)
                              | bold;
          else
            header = header | bgcolor(Color::RGB(25,25,25))
                              | color(Color::Cyan)
                              | bold;

          rows.push_back(header);
        }
      }

      return vbox(rows);
    });

  album_scroller =
    Scroller(album_content, &selected_album_index,
             Color::Green, Color::GrayDark);

  container = Container::Horizontal({
    artist_menu,
    album_scroller
  });

  container |= CatchEvent(
    [&](Event e)
    {
      if (e == Event::Character('\t'))
      {
        focus_on_artists = !focus_on_artists;
        focus_on_artists
          ? artist_menu->TakeFocus()
          : album_scroller->TakeFocus();
        return true;
      }

      if (!focus_on_artists)
      {
        if (e == Event::ArrowDown)
        {
          moveSelection(1);
          return true;
        }
        if (e == Event::ArrowUp)
        {
          moveSelection(-1);
          return true;
        }
      }

      if (e == Event::Character('q'))
      {
        ScreenInteractive::Active()->Exit();
        return true;
      }

      if (!m_audio)
        return false;

      if (e == Event::Character('p'))
      {
        m_audio->isPlaying()
          ? m_audio->pauseCurrent()
          : m_audio->playCurrent();
        return true;
      }

      if (e == Event::Character('n'))
      {
        m_audio->nextTrack();
        m_mpris->updateMetadata();
        m_mpris->notify();
        return true;
      }

      if (e == Event::Character('b'))
      {
        m_audio->previousTrack();
        m_mpris->updateMetadata();
        m_mpris->notify();
        return true;
      }

      if (e == Event::Character('j'))
      {
        m_pendingSeek -= 2;
        return true;
      }

      if (e == Event::Character('k'))
      {
        m_pendingSeek += 2;
        return true;
      }

      if (e == Event::Character('='))
      {
        m_audio->setVolume(
          std::min(1.5f, m_audio->getVolume() + 0.05f));
        return true;
      }

      if (e == Event::Character('-'))
      {
        m_audio->setVolume(
          std::max(0.0f, m_audio->getVolume() - 0.05f));
        return true;
      }

      if (e == Event::Character('2'))
      {
        active_screen = UIScreen::NowPlaying;
        lyrics_scroller->TakeFocus();
        return true;
      }

      if (e == Event::Escape ||
          e == Event::Character('1'))
      {
        active_screen = UIScreen::Main;
        return true;
      }

      if (e == Event::Return)
      {
        playSelected();
        return true;
      }

      return false;
    });

  renderer = Renderer(container,
    [&]() -> Element
    {
      return renderRoot();
    });
}

/* ================= RENDER ROOT ================= */

auto Interface::renderRoot() -> Element
{
  switch (active_screen)
  {
    case UIScreen::Main:       return renderMain();
    case UIScreen::NowPlaying: return renderNowPlaying();
    case UIScreen::Help:       return renderHelp();
    case UIScreen::Config:     return renderConfig();
    case UIScreen::Queue:      return renderQueue();
  }
  return text("");
}

/* ================= MAIN ================= */

auto Interface::renderMain() -> Element
{
  if (selected_artist != current_artist_cache)
    rebuildAlbumsForSelectedArtist();

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  auto artist_inner =
    window(text(" Artists ") | bold,
           artist_menu->Render() | frame | flex)
    | size(WIDTH, EQUAL, half_width);

  auto album_inner =
    window(text(" Albums ") | bold,
           album_scroller->Render())
    | size(WIDTH, EQUAL, term.dimx - half_width);

  auto artist_pane =
    artist_inner | border
    | color(focus_on_artists ? Color::Green
                             : Color::GrayDark);

  auto album_pane =
    album_inner | border
    | color(!focus_on_artists ? Color::Green
                              : Color::GrayDark);

  return vbox({
    renderTitleBar(),
    hbox({artist_pane, album_pane}) | flex,
    renderStatusBar() | size(HEIGHT, EQUAL, 3)
  });
}

/* ================= NOW PLAYING ================= */

void Interface::loadLyrics()
{
  lyrics_lines.clear();
  selected_lyric_index = 0;

  if (!m_audio)
  {
    lyrics_lines.emplace_back("Lyrics not available");
    return;
  }

  auto meta = m_audio->getCurrentMetadata();
  if (!meta)
  {
    lyrics_lines.emplace_back("Lyrics not available");
    return;
  }

  std::string raw_lyrics;

  if (!meta->lyrics.empty())
  {
    raw_lyrics = meta->lyrics;
  }
  else
  {
    auto cachePath =
      helpers::lrc::genLRCFilePath(
        meta->artist,
        meta->title,
        meta->album);

    if (auto cached =
          helpers::lrc::tryReadCachedLRC(cachePath))
    {
      LOG_INFO("Loaded lyrics from cache: {}",
               cachePath.string());
      raw_lyrics = *cached;
    }
  }

  if (raw_lyrics.empty())
  {
    lyrics_lines.emplace_back("Lyrics not available");
    return;
  }

  std::stringstream ss(raw_lyrics);
  std::string line;

  while (std::getline(ss, line))
  {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    lyrics_lines.push_back(line);
  }

  if (lyrics_lines.empty())
    lyrics_lines.emplace_back("Lyrics not available");
}

auto Interface::renderNowPlaying() -> Element
{
  if (!m_audio)
    return text("No Track");

  auto meta      = m_audio->getCurrentMetadata();
  auto trackInfo = m_audio->getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("No Track");

  static PathStr last_art_path;

  if (meta->artUrl != last_art_path)
  {
    last_art_path = meta->artUrl;
    loadAlbumArt(
      utils::fs::fromAbsFilePathUri(
        meta->artUrl.c_str()).c_str());
    loadLyrics();
  }

  auto term = Terminal::Size();
  int half_width     = term.dimx / 2;
  int content_height = term.dimy - 4;

  auto left =
    renderAlbumArt()
    | size(WIDTH, EQUAL, half_width)
    | size(HEIGHT, EQUAL, content_height)
    | border;

  auto lyrics_view =
    window(text(" Lyrics ") | bold,
           lyrics_scroller->Render()
           | frame | flex)
    | size(WIDTH, EQUAL, term.dimx - half_width)
    | size(HEIGHT, EQUAL, content_height)
    | border;

  return vbox({
    renderTitleBar(),
    hbox({left, lyrics_view}) | flex,
    renderStatusBar() | size(HEIGHT, EQUAL, 3)
  })
  | bgcolor(Color::RGB(18,18,18));
}

/* ================= IMAGE LOADING ================= */

void Interface::loadAlbumArt(const std::string& path)
{
  if (album_loaded)
  {
    album_pixels.clear();
    album_loaded = false;
  }

  int desired_channels = 3;

  unsigned char* data =
    stbi_load(path.c_str(),
              &album_w,
              &album_h,
              &album_channels,
              desired_channels);

  if (!data)
  {
    album_loaded = false;
    return;
  }

  if (album_w <= 0 || album_h <= 0)
  {
    stbi_image_free(data);
    album_loaded = false;
    return;
  }

  size_t data_size = album_w * album_h * desired_channels;
  album_pixels.resize(data_size);
  std::memcpy(album_pixels.data(), data, data_size);

  stbi_image_free(data);
  album_loaded = true;
}

/* ================= IMAGE PROCESSING ================= */

Color Interface::getAverageColor(int start_x,
                                 int start_y,
                                 int width,
                                 int height)
{
  if (!album_loaded || width <= 0 || height <= 0)
    return Color::Black;

  long r_sum = 0;
  long g_sum = 0;
  long b_sum = 0;
  int  count = 0;

  int end_x = std::min(start_x + width, album_w);
  int end_y = std::min(start_y + height, album_h);

  start_x = std::max(0, start_x);
  start_y = std::max(0, start_y);

  for (int y = start_y; y < end_y; ++y)
  {
    for (int x = start_x; x < end_x; ++x)
    {
      int idx = (y * album_w + x) * 3;

      r_sum += album_pixels[idx + 0];
      g_sum += album_pixels[idx + 1];
      b_sum += album_pixels[idx + 2];

      count++;
    }
  }

  if (count == 0)
    return Color::Black;

  return Color::RGB(
    static_cast<uint8_t>(r_sum / count),
    static_cast<uint8_t>(g_sum / count),
    static_cast<uint8_t>(b_sum / count));
}

/* ================= IMAGE RENDERING ================= */

Element Interface::renderAlbumArt()
{
  const int constraint_w = 70;
  const int constraint_h = 70;

  if (!album_loaded)
  {
    return text("No Cover")
           | borderRounded
           | size(WIDTH, EQUAL, constraint_w)
           | size(HEIGHT, EQUAL, constraint_h)
           | center;
  }

  int virtual_max_h = constraint_h * 2;

  float scale_w = (float)constraint_w / album_w;
  float scale_h = (float)virtual_max_h / album_h;
  float scale   = std::min(scale_w, scale_h);

  int render_w = static_cast<int>(album_w * scale);
  int render_h = static_cast<int>(album_h * scale);

  if (render_h % 2 != 0)
    render_h--;

  if (render_w <= 0 || render_h <= 0)
    return text("Image too small");

  Elements rows;

  float src_block_w = (float)album_w / render_w;
  float src_block_h = (float)album_h / render_h;

  for (int y = 0; y < render_h; y += 2)
  {
    Elements row_cells;

    int horizontal_padding =
      (constraint_w - render_w) / 2;

    if (horizontal_padding > 0)
      row_cells.push_back(
        text(std::string(horizontal_padding, ' ')));

    for (int x = 0; x < render_w; ++x)
    {
      int src_x     = static_cast<int>(x * src_block_w);
      int src_y_top = static_cast<int>(y * src_block_h);
      int src_y_bot = static_cast<int>((y + 1) * src_block_h);

      int w = std::max(1,
        static_cast<int>(src_block_w));
      int h = std::max(1,
        static_cast<int>(src_block_h));

      Color top_color =
        getAverageColor(src_x, src_y_top, w, h);

      Color bot_color =
        getAverageColor(src_x, src_y_bot, w, h);

      row_cells.push_back(
        text("▀")
        | color(top_color)
        | bgcolor(bot_color));
    }

    rows.push_back(hbox(row_cells));
  }

  int used_rows = render_h / 2;
  int remaining = constraint_h - used_rows;

  for (int p = 0; p < remaining; ++p)
  {
    rows.push_back(
      text(std::string(constraint_w, ' ')));
  }

  return vbox(rows) | center;
}

/* ================= STATUS BAR ================= */

auto Interface::renderStatusBar() -> Element
{
  if (!m_audio)
    return text("");

  auto meta      = m_audio->getCurrentMetadata();
  auto trackInfo = m_audio->getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("");

  float progress = 0.f;
  if (meta->duration > 0)
    progress =
      (float)trackInfo->positionSec /
      meta->duration;

  int bar_width = 36;
  int filled    =
    static_cast<int>(progress * bar_width);

  static int tick = 0;
  tick++;

  int glow_pos = 0;
  if (filled > 0)
    glow_pos = (tick / 2) % filled;

  Elements bar;

  for (int i = 0; i < bar_width; ++i)
  {
    if (i < filled)
    {
      if (std::abs(i - glow_pos) <= 1)
        bar.push_back(text("█")
          | color(Color::GreenLight)
          | bold);
      else
        bar.push_back(text("█")
          | color(Color::Green));
    }
    else
    {
      bar.push_back(text("░")
        | color(Color::GrayDark));
    }
  }

  auto progress_bar = hbox(bar);

  return hbox({
    text(" "),
    text(m_audio->isPlaying()
      ? " PLAY " : " PAUSE ")
      | (m_audio->isPlaying()
          ? color(Color::GreenLight)
          : color(Color::RedLight))
      | bold,
    text("  "),
    separatorLight(),
    text("  "),
    text(meta->title) | bold,
    text("  •  ") | dim,
    text(meta->artist) | color(Color::Cyan),
    text("  •  ") | dim,
    text(meta->genre.empty()
      ? "Unknown" : meta->genre)
      | color(Color::MagentaLight),
    text("  "),
    separatorLight(),
    text("  "),
    progress_bar,
    text("  "),
    text(utils::timer::fmtTime(
      trackInfo->positionSec))
      | color(Color::GreenLight),
    text(" / ") | dim,
    text(utils::timer::fmtTime(
      meta->duration))
      | dim,
    filler(),
    text(std::to_string(meta->bitrate)
         + " kbps")
      | dim | color(Color::Cyan),
    text("  "),
    separatorLight(),
    text("  "),
    text("Vol ") | dim,
    text(std::to_string(
      static_cast<int>(
        m_audio->getVolume() * 100)))
      | dim,
    text(" "),
    gauge(m_audio->getVolume())
      | size(WIDTH, EQUAL, 10)
      | color(Color::BlueLight),
    text("   ")
  })
  | bgcolor(Color::RGB(18,18,18))
  | borderRounded;
}

/* ================= THREAD LOOPS ================= */

void Interface::runStatusLoop(audio::Service& audio)
{
  while (m_isRunning.load())
  {
    helpers::telemetry::updateTelemetryProgress(
      audio,
      m_currentPlay,
      m_lastPlayTick);

    if (auto info =
          audio.getCurrentTrackInfo())
    {
      const auto pos = info->positionSec;
      const auto len = info->lengthSec;

      if (len > 0 && pos >= len)
      {
        helpers::telemetry::playbackTransition(
          audio,
          m_telemetryCtx,
          m_currentPlay,
          m_lastPlayTick,
          [&]() -> void
          {
            audio.nextTrack();
          });

        m_mpris->updateMetadata();
        m_mpris->notify();
      }
    }

    if (autoNextIfFinished(
          audio,
          *m_mpris,
          m_lastAutoNextTid,
          m_autoNextInProgress))
    {
      helpers::telemetry::playbackTransition(
        audio,
        m_telemetryCtx,
        m_currentPlay,
        m_lastPlayTick,
        []() -> void{});
    }

    if (m_screen)
      m_screen->PostEvent(Event::Custom);

    std::this_thread::sleep_for(
      std::chrono::milliseconds(100));
  }
}

void Interface::runMprisLoop()
{
  while (m_isRunning.load())
  {
    std::this_thread::sleep_for(
      std::chrono::milliseconds(100));
    m_mpris->poll();
  }
}

void Interface::runSeekLoop(audio::Service& audio)
{
  while (m_isRunning.load())
  {
    double d = m_pendingSeek.exchange(0.0);

    if (std::abs(d) > 0.01)
    {
      if (d > 0.0)
        audio.seekForward(d);
      else
        audio.seekBackward(-d);
    }

    m_mpris->notify();

    std::this_thread::sleep_for(
      std::chrono::milliseconds(50));
  }
}

/* ================= PLAY SELECTED ================= */

void Interface::playSelected()
{
  if (!m_audio)
    return;

  m_audio->clearPlaylist();

  if (selected_album_index < 0 ||
    selected_album_index >= (int)view_song_objects.size())
  return;

  auto songObj = view_song_objects[selected_album_index];
  if (!songObj)
    return;

  auto handle = m_audio->registerTrack(songObj);
  m_audio->addToPlaylist(handle);
  m_audio->nextTrack();


  query::songmap::read::forEachSongInAlbum(
  *m_songMapTS, m_audio->getCurrentMetadata()->artist, m_audio->getCurrentMetadata()->album,
  [&](const Disc&, const Track&, const ino_t, const std::shared_ptr<Song>& song) -> void
  {
    if (song->metadata.track <= m_audio->getCurrentMetadata()->track)
      return;

    auto h = m_audio->registerTrack(song);
    m_audio->addToPlaylist(h);
  });

  m_mpris->updateMetadata();
  m_mpris->notify();
}

/* ================= HELP ================= */

auto Interface::renderHelp() -> Element
{
  return text("Help screen (to implement)");
}

/* ================= CONFIG ================= */

auto Interface::renderConfig() -> Element
{
  return text("Config screen (to implement)");
}

/* ================= QUEUE ================= */

auto Interface::renderQueue() -> Element
{
  return text("Queue screen (to implement)");
}

/* ================= MOVE SELECTION ================= */

void Interface::moveSelection(int delta)
{
  int new_index = selected_album_index;

  do
  {
    new_index += delta;

    if (new_index < 0)
      new_index = 0;

    if (new_index >= (int)view_song_objects.size())
      new_index = view_song_objects.size() - 1;

  } while (!view_song_objects[new_index] &&
           new_index > 0 &&
           new_index < (int)view_song_objects.size() - 1);

  if (view_song_objects[new_index])
    selected_album_index = new_index;
}

/* ================= TITLE BAR ================= */

auto Interface::renderTitleBar() -> Element
{
  return hbox({
           text(" inLimbo ") | bold,
           filler(),
           text("Modern TUI Player") | dim
         })
         | bgcolor(Color::Blue)
         | color(Color::White);
}

} // namespace frontend::tui
