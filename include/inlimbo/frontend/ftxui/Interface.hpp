#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "audio/Service.hpp"
#include "config/Watcher.hpp"
#include "mpris/Service.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"

#include <atomic>
#include <optional>
#include <string>
#include <vector>

namespace frontend::tui
{

enum class UIScreen
{
  Main,
  NowPlaying,
  Help,
  Config,
  Queue
};

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>* songMap,
                     telemetry::Context* telemetry,
                     mpris::Service* mprisService);

  void run(audio::Service& audio);

  IMMUTABLE(Interface);

  auto getRoot() -> ftxui::Component;

private:
  /* ================= BACKEND ================= */

  config::Watcher            m_cfgWatcher;
  threads::SafeMap<SongMap>* m_songMapTS{nullptr};
  telemetry::Context*        m_telemetryCtx{nullptr};
  mpris::Service*            m_mpris{nullptr};
  audio::Service*            m_audio{nullptr};
  ftxui::ScreenInteractive*  m_screen{nullptr};

  /* ================= STATE ================= */

  UIScreen active_screen{UIScreen::Main};

  int                      selected_album_index = 0;
  int                      selected_artist      = 0;
  int                      current_artist_cache = 0;
  std::vector<std::string> lyrics_lines;
  int                      selected_lyric_index = 0;
  bool                     focus_on_artists     = true;

  std::vector<Artist>                artists;
  std::vector<std::string>           album_view_lines;
  std::vector<std::shared_ptr<Song>> view_song_objects;

  std::atomic<bool>  m_isRunning{false};
  std::optional<i64> m_lastPlayTick;

  std::atomic<double> m_pendingSeek{0.0};
  std::atomic<ui8>    m_lastAutoNextTid{0};
  std::atomic<bool>   m_autoNextInProgress{false};

  std::optional<telemetry::Event> m_currentPlay;

  /* ================= FTXUI ================= */

  ftxui::Component artist_menu;
  ftxui::Component album_content;
  ftxui::Component album_scroller;
  ftxui::Component container;
  ftxui::Component renderer;
  ftxui::Component lyrics_menu;
  ftxui::Component lyrics_scroller;

  /* ================= IMAGE DATA ================= */

  std::vector<unsigned char> album_pixels;
  int                        album_w        = 0;
  int                        album_h        = 0;
  int                        album_channels = 0;
  bool                       album_loaded   = false;

  /* ================= METHODS ================= */

  void rebuildLibrary();
  void rebuildAlbumsForSelectedArtist();
  void buildAlbumViewForArtist(const Artist& artist);

  void createComponents();
  void moveSelection(int delta);
  void playSelected();

  auto renderRoot() -> ftxui::Element;
  auto renderMain() -> ftxui::Element;
  auto renderNowPlaying() -> ftxui::Element;
  auto renderStatusBar() -> ftxui::Element;
  auto renderTitleBar() -> ftxui::Element;
  auto renderHelp() -> ftxui::Element;
  auto renderConfig() -> ftxui::Element;
  auto renderQueue() -> ftxui::Element;

  void loadLyrics();
  void loadAlbumArt(const std::string& path);

  auto getAverageColor(int start_x, int start_y,
                               int width, int height) -> ftxui::Color;

  auto renderAlbumArt() -> ftxui::Element;

  void runStatusLoop(audio::Service& audio);
  void runMprisLoop();
  void runSeekLoop(audio::Service& audio);
};

} // namespace frontend::tui
