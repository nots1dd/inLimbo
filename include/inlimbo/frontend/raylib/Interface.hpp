#pragma once

#include "audio/Service.hpp"
#include "config/Watcher.hpp"
#include "frontend/raylib/Structs.hpp"
#include "frontend/raylib/input/Handler.hpp"
#include "frontend/raylib/ui/Fonts.hpp"
#include "frontend/raylib/view/AlbumsView.hpp"
#include "frontend/raylib/view/ArtistsView.hpp"
#include "frontend/raylib/view/Header.hpp"
#include "frontend/raylib/view/MetadataOverlay.hpp"
#include "frontend/raylib/view/NowPlaying.hpp"
#include "frontend/raylib/view/StatusBar.hpp"
#include "mpris/Service.hpp"
#include "thread/Map.hpp"
#include "utils/PathResolve.hpp"
#include "utils/Snapshot.hpp"

static constexpr cstr FRONTEND_NAME = "raylib";

namespace frontend::raylib
{

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>* songMap, mpris::Service* mprisService)
      : m_cfgWatcher(utils::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME)),
        m_songMap(songMap), m_mpris(mprisService)
  {
  }

  void run(audio::Service& audio);

private:
  config::Watcher            m_cfgWatcher;
  threads::SafeMap<SongMap>* m_songMap{nullptr};
  mpris::Service*            m_mpris{nullptr};

  // config stuff
  utils::Snapshot<RaylibConfig> m_cfg{};

  void loadConfig();
  void draw(audio::Service& audio);
  void statusLoop(audio::Service& audio);

  state::UI      m_ui;
  state::Library m_library;
  ui::Fonts      m_fonts;
  input::Handler m_input;

  view::Header          m_header;
  view::ArtistsView     m_artists;
  view::AlbumsView      m_albums;
  view::NowPlaying      m_nowPlaying;
  view::StatusBar       m_status;
  view::MetadataOverlay m_metaOverlay;
};

} // namespace frontend::raylib
