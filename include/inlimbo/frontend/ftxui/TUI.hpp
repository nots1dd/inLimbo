#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "audio/Service.hpp"
#include "components/scroll/Scrollable.hpp"
#include "query/SongMap.hpp"
#include "thread/Map.hpp"
#include "utils/timer/Timer.hpp"

#include <string>
#include <vector>

#include <stb_image.h>

namespace frontend::tui
{

using namespace ftxui;

/* ========================= SCREENS ========================= */

enum class UIScreen
{
  Main,
  NowPlaying,
  Help,
  Config,
  Queue
};

class TuiRenderer
{
public:
  explicit TuiRenderer(threads::SafeMap<SongMap>* songMap, audio::Service& audio)
      : m_songMapTS(songMap), m_audio(audio)
  {
    rebuildLibrary();
    current_artist_cache = selected_artist;
    createComponents();
  }

  auto getRoot() -> Component { return renderer; }

private:
  /* ================= BACKEND ================= */

  threads::SafeMap<SongMap>* m_songMapTS{nullptr};
  audio::Service&            m_audio;

  /* ================= STATE ================= */

  UIScreen active_screen{UIScreen::Main};

  int  selected_album_index = 0;
  int  selected_artist      = 0;
  int  current_artist_cache = 0;
  bool focus_on_artists     = true;

  std::vector<Artist>                artists;
  std::vector<std::string>           album_view_lines;
  std::vector<std::shared_ptr<Song>> view_song_objects;

  /* ================= FTXUI ================= */

  Component artist_menu;
  Component album_content;
  Component album_scroller;
  Component container;
  Component renderer;

  std::vector<unsigned char> album_pixels;
  int                        album_w      = 0;
  int                        album_h      = 0;
  bool                       album_loaded = false;

  /* ========================================================= */
  /* ================= LIBRARY BUILD ========================= */
  /* ========================================================= */

  void rebuildLibrary()
  {
    artists.clear();
    album_view_lines.clear();
    view_song_objects.clear();

    if (!m_songMapTS)
      return;

    query::songmap::read::forEachArtist(*m_songMapTS,
                                        [&](const Artist& artist, const AlbumMap&) -> void
                                        { artists.push_back(artist); });

    if (!artists.empty())
      buildAlbumViewForArtist(artists[0]);
  }

  void rebuildAlbumsForSelectedArtist()
  {
    if (selected_artist < 0 || selected_artist >= (int)artists.size())
      return;

    buildAlbumViewForArtist(artists[selected_artist]);

    selected_album_index = 0;
    current_artist_cache = selected_artist;
  }

  void buildAlbumViewForArtist(const Artist& artist)
  {
    album_view_lines.clear();
    view_song_objects.clear();

    Album current_album;
    Disc  current_disc = -1;

    query::songmap::read::forEachSong(
      *m_songMapTS,
      [&](const Artist& a, const Album& album, const Disc disc, const Track track, const ino_t,
          const std::shared_ptr<Song>& song) -> void
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
          (track < 10 ? "0" : "") + std::to_string(track) + "  " + song->metadata.title;

        album_view_lines.push_back(line);
        view_song_objects.push_back(song);
      });
  }

  /* ========================================================= */
  /* ================= COMPONENT SETUP ======================= */
  /* ========================================================= */

  void createComponents()
  {
    artist_menu = Menu(&artists, &selected_artist);

    album_content = Renderer(
      [&]() -> Element
      {
        Elements rows;

        for (size_t i = 0; i < album_view_lines.size(); ++i)
        {
          const auto& line    = album_view_lines[i];
          auto        songObj = view_song_objects[i];

          bool selected = (int)i == selected_album_index;

          Element selector = selected ? text("➤ ") | color(Color::Yellow) : text("  ");

          if (songObj)
          {
            std::string dur = utils::timer::fmtTime(songObj->metadata.duration);

            auto row = hbox({selector, text(line), filler(), text(dur) | dim});

            if (m_audio.isPlaying())
            {
              auto meta = m_audio.getCurrentMetadata();
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
              header = header | bgcolor(Color::RGB(35, 35, 35)) | color(Color::YellowLight) | bold;
            else
              header = header | bgcolor(Color::RGB(25, 25, 25)) | color(Color::Cyan) | bold;

            rows.push_back(header);
          }
        }

        return vbox(rows);
      });

    album_scroller = Scroller(album_content, &selected_album_index, Color::Green, Color::GrayDark);

    container = Container::Horizontal({artist_menu, album_scroller});

    container |= CatchEvent(
      [&](Event e) -> bool
      {
        /* ---------- TAB: Switch Focus ---------- */

        if (e == Event::Character('\t'))
        {
          focus_on_artists = !focus_on_artists;

          if (focus_on_artists)
            artist_menu->TakeFocus();
          else
            album_scroller->TakeFocus();

          return true;
        }

        /* ---------- Artist Selection Changed ---------- */

        if (focus_on_artists && (e == Event::ArrowUp || e == Event::ArrowDown ||
                                 e == Event::PageUp || e == Event::PageDown))
        {
          // Defer rebuild until next frame.
          return false;
        }

        /* ---------- Navigation ---------- */

        if (e == Event::Character('n'))
        {
          active_screen = UIScreen::NowPlaying;
          return true;
        }

        if (e == Event::Character('?'))
        {
          active_screen = UIScreen::Help;
          return true;
        }

        if (e == Event::Character('c'))
        {
          active_screen = UIScreen::Config;
          return true;
        }

        if (e == Event::Escape)
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

    renderer = Renderer(container, [&]() -> Element { return renderRoot(); });
  }

  /* ========================================================= */
  /* ================= PLAY SELECTED ========================= */
  /* ========================================================= */

  void playSelected()
  {
    if (selected_album_index < 0 || selected_album_index >= (int)view_song_objects.size())
      return;

    auto songObj = view_song_objects[selected_album_index];
    if (!songObj)
      return;

    auto handle = m_audio.registerTrack(songObj);
    m_audio.addToPlaylist(handle);
    m_audio.nextTrack();
  }

  /* ========================================================= */
  /* ================= ROOT RENDER =========================== */
  /* ========================================================= */

  auto renderRoot() -> Element
  {
    switch (active_screen)
    {
      case UIScreen::Main:
        return renderMain();
      case UIScreen::NowPlaying:
        return renderNowPlaying();
      case UIScreen::Help:
        return renderHelp();
      case UIScreen::Config:
        return renderConfig();
      case UIScreen::Queue:
        return renderQueue();
    }
    return text("");
  }

  /* ========================================================= */
  /* ================= MAIN ================================= */
  /* ========================================================= */

  auto renderMain() -> Element
  {
    if (selected_artist != current_artist_cache)
      rebuildAlbumsForSelectedArtist();

    auto term       = Terminal::Size();
    int  half_width = term.dimx / 2;

    auto artist_pane = window(text(" Artists ") | bold, artist_menu->Render() | frame | flex) |
                       size(WIDTH, EQUAL, half_width) | border |
                       color(focus_on_artists ? Color::Green : Color::GrayDark);

    auto album_pane = window(text(" Albums ") | bold, album_scroller->Render()) |
                      size(WIDTH, EQUAL, term.dimx - half_width) | border |
                      color(!focus_on_artists ? Color::Green : Color::GrayDark);

    return vbox({renderTitleBar(), hbox({artist_pane, album_pane}), renderStatusBar()});
  }

  /* ========================================================= */
  /* ================= NOW PLAYING =========================== */
  /* ========================================================= */

  auto renderNowPlaying() -> Element
  {
    auto meta      = m_audio.getCurrentMetadata();
    auto trackInfo = m_audio.getCurrentTrackInfo();

    if (!meta || !trackInfo)
      return text("No Track");

    static PathStr last_art_path;

    if (meta->artUrl != last_art_path)
    {
      last_art_path = meta->artUrl;
      loadAlbumArt(meta->artUrl);
    }

    float progress = 0.f;
    if (trackInfo->lengthSec > 0)
      progress = (float)trackInfo->positionSec / trackInfo->lengthSec;

    int remaining = std::max(0.0, trackInfo->lengthSec - trackInfo->positionSec);

    return vbox({renderTitleBar(), separator(),
                 vbox({separatorEmpty(), renderAlbumArt(), separatorEmpty(),
                       text(meta->title) | bold | center,
                       text(meta->artist + " - " + meta->album) | dim | center, separator(),
                       gauge(progress) | size(WIDTH, EQUAL, 60) | color(Color::GreenLight) | center,
                       hbox({text(utils::timer::fmtTime(trackInfo->positionSec)), filler(),
                             text(utils::timer::fmtTime(trackInfo->lengthSec) + " (" +
                                  utils::timer::fmtTime(remaining) + " left)") |
                               dim}) |
                         size(WIDTH, EQUAL, 60) | center,
                       separatorEmpty()}) |
                   flex | center,
                 renderStatusBar()}) |
           bgcolor(Color::RGB(18, 18, 18));
  }

  /* ========================================================= */
  /* ================= STATUS BAR ============================ */
  /* ========================================================= */

  auto renderStatusBar() -> Element
  {
    auto meta      = m_audio.getCurrentMetadata();
    auto trackInfo = m_audio.getCurrentTrackInfo();

    if (!meta || !trackInfo)
      return text("");

    float progress = 0.f;
    if (meta->duration > 0)
      progress = (float)trackInfo->positionSec / meta->duration;

    /* ================= Animated Glow ================= */

    int bar_width = 36;
    int filled    = static_cast<int>(progress * bar_width);

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
          bar.push_back(text("█") | color(Color::GreenLight) | bold);
        else
          bar.push_back(text("█") | color(Color::Green));
      }
      else
      {
        bar.push_back(text("░") | color(Color::GrayDark));
      }
    }

    auto progress_bar = hbox(bar);

    /* ================= Layout ================= */

    return hbox(
             {text(" "),
              text(m_audio.isPlaying() ? " PLAY " : " PAUSE ") |
                (m_audio.isPlaying() ? color(Color::GreenLight) : color(Color::RedLight)) | bold,
              text("  "),
              separatorLight(),
              text("  "),
              text(meta->title) | bold,
              text("  •  ") | dim,
              text(meta->artist) | color(Color::Cyan),
              text("  "),
              separatorLight(),
              text("  "),
              progress_bar,
              text("  "),
              text(utils::timer::fmtTime(trackInfo->positionSec)) | color(Color::GreenLight),
              text(" / ") | dim,
              text(utils::timer::fmtTime(meta->duration)) | dim,
              filler(),
              text(std::to_string(meta->bitrate) + " kbps") | dim | color(Color::Cyan),
              text("  "),
              separatorLight(),
              text("  "),
              text("Vol ") | dim,
              gauge(m_audio.getVolume() / 100.f) | size(WIDTH, EQUAL, 10) | color(Color::BlueLight),
              text(" ")}) |
           bgcolor(Color::RGB(18, 18, 18)) | borderRounded;
  }

  void loadAlbumArt(const std::string& path)
  {
    int            channels;
    unsigned char* data = stbi_load(path.c_str(), &album_w, &album_h, &channels, 3);

    if (!data)
    {
      album_loaded = false;
      return;
    }

    album_pixels.assign(data, data + album_w * album_h * 3);
    stbi_image_free(data);
    album_loaded = true;
  }

  /* ========================================================= */

  auto renderTitleBar() -> Element
  {
    return hbox({text(" inLimbo ") | bold, filler(), text("Modern TUI Player") | dim}) |
           bgcolor(Color::Blue) | color(Color::White);
  }

  auto renderHelp() -> Element { return text("Help screen (to implement)"); }

  auto renderConfig() -> Element { return text("Config screen (to implement)"); }

  auto renderQueue() -> Element { return text("Queue screen (to implement)"); }

  auto renderAlbumArt() -> Element
  {
    if (!album_loaded)
    {
      return text("No Cover") | borderRounded | size(WIDTH, EQUAL, 40) | size(HEIGHT, EQUAL, 12) |
             center;
    }

    const int render_w = 40;
    const int render_h = 20;

    Canvas c(render_w, render_h);

    float sx = album_w / (float)render_w;
    float sy = album_h / (float)render_h;

    for (int y = 0; y < render_h; ++y)
    {
      for (int x = 0; x < render_w; ++x)
      {
        int src_x = std::min((int)(x * sx), album_w - 1);
        int src_y = std::min((int)(y * sy), album_h - 1);
        int idx   = (src_y * album_w + src_x) * 3;

        c.DrawPoint(
          x, y, true,
          Color::RGB(album_pixels[idx + 0], album_pixels[idx + 1], album_pixels[idx + 2]));
      }
    }

    return canvas(c) | borderRounded | center;
  }
};

} // namespace frontend::tui
