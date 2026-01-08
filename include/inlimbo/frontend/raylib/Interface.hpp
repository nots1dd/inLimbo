#pragma once

#include <atomic>
#include <raylib.h>
#include <vector>

#include "audio/Service.hpp"
#include "mpris/Service.hpp"
#include "thread/Map.hpp"

namespace frontend::raylib
{

enum class Screen
{
  Library,    // current artist/album browser
  NowPlaying, // full-screen player view
};

static constexpr Color BG_MAIN   = {18, 18, 18, 255};
static constexpr Color BG_PANEL  = {28, 28, 28, 255};
static constexpr Color ACCENT    = {90, 170, 255, 255};
static constexpr Color TEXT_MAIN = {235, 235, 235, 255};
static constexpr Color TEXT_DIM  = {150, 150, 150, 255};

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>* songMap, mpris::Service* mprisService)
      : m_songMapTS(songMap), m_mprisService(mprisService)
  {
  }

  void run(audio::Service& audio);

private:
  void rebuildArtists();
  void rebuildAlbums();

  void handleInput(audio::Service& audio);
  void drawNowPlaying(audio::Service& audio);
  void drawPlaybackControls(audio::Service& audio);
  void drawMetadataOverlay(const Metadata& meta);
  void drawCenteredArt(const Metadata& meta);
  void drawProgressBar(const audio::service::TrackInfo& info, audio::Service& audio);
  void draw(audio::Service& audio);

  void drawArtistsPane(audio::Service& audio);
  void drawAlbumsPane(audio::Service& audio);
  void drawStatusBar(audio::Service& audio);
  void drawArt(const Metadata& meta);

private:
  threads::SafeMap<SongMap>* m_songMapTS;
  mpris::Service*            m_mprisService{nullptr};

  std::atomic<bool> m_isRunning{false};

  std::vector<Artist> m_artists;
  std::vector<Album>  m_albums;
  bool                m_showMetaInfo      = false;
  bool                m_trackEndedHandled = false;

  void playSongWithAlbumQueue(audio::Service& audio, const Song& song);
  auto truncateText(const char* text, float maxWidth, float fontSize) -> std::string;
  void drawTextTruncated(Font font, const char* text, Vector2 pos, float fontSize, float spacing,
                         Color color, float maxWidth);
  void drawTextTruncatedCentered(Font font, const char* text, float centerX, float y,
                                 float fontSize, float spacing, Color color, float maxWidth);
  void drawHeader(const char* title);

  int m_selArtist{0};
  int m_selAlbum{0};

  bool m_dirty{true};
  bool m_showArt{false};

  Texture2D   m_artTex{};
  bool        m_artLoaded{false};
  std::string m_loadedArtUrl = {};

  Font m_fontRegular{};
  Font m_fontBold{};

  float m_screenBlend = 0.0f; // 0 = library, 1 = player
  float m_artistScrollY;
  float m_albumScrollY;

  Screen m_screen  = Screen::Library;
  bool   m_seeking = false;
};

} // namespace frontend::raylib
