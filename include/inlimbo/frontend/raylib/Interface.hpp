#pragma once

#include <atomic>
#include <raylib.h>
#include <vector>

#include "audio/Service.hpp"
#include "mpris/Service.hpp"
#include "thread/Map.hpp"

namespace frontend::raylib
{

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>& songMap, mpris::Service* mprisService = nullptr);

  void run(audio::Service& audio);

private:
  void rebuildArtists();
  void rebuildAlbums();

  void handleInput(audio::Service& audio);
  void draw(audio::Service& audio);

  void drawArtistsPane();
  void drawAlbumsPane(audio::Service& audio);
  void drawStatusBar(audio::Service& audio);
  void drawArt(const Metadata& meta);

private:
  threads::SafeMap<SongMap>& m_songMapTS;
  mpris::Service*            m_mprisService{nullptr};

  std::atomic<bool> m_isRunning{false};

  std::vector<Artist> m_artists;
  std::vector<Album>  m_albums;

  int m_selArtist{0};
  int m_selAlbum{0};

  bool m_dirty{true};
  bool m_showArt{false};

  Texture2D m_artTex{};
  bool      m_artLoaded{false};
};

} // namespace frontend::raylib
