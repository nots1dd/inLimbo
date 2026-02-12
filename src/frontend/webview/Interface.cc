#include "Logger.hpp"
// TagLib and gdx (for webview) both contain
// Bool as a conflicting type so they need to
// be included in order
// clang-format off
#include "query/SongMap.hpp"
#include "frontend/webview/Interface.hpp"
// clang-format on
#include "utils/fs/FileUri.hpp"
#include "utils/string/Equals.hpp"

namespace frontend::webview
{

static auto autoNextIfFinished(audio::Service& audio, mpris::Service& mpris,
                               std::atomic<ui8>& lastTid, std::atomic<bool>& inProgress) -> bool
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

  // Already handled this track
  if (info.tid == lastTid.load(std::memory_order_relaxed))
    return false;

  bool expected = false;
  if (!inProgress.compare_exchange_strong(expected, true))
    return false;

  lastTid.store(info.tid, std::memory_order_relaxed);

  audio.nextTrackGapless();
  mpris.updateMetadata();
  mpris.notify();

  LOG_DEBUG("Gapless next invoked.");

  inProgress.store(false, std::memory_order_release);
  return true;
}

static void playSongWithAlbumQueue(audio::Service& audio, threads::SafeMap<SongMap>& songs,
                                   const std::shared_ptr<Song>& song, mpris::Service& mpris)
{
  audio.clearPlaylist();

  auto h = audio.registerTrack(song);
  audio.addToPlaylist(h);
  audio.nextTrack();

  query::songmap::read::forEachSongInAlbum(
    songs, song->metadata.artist, song->metadata.album,
    [&](const Disc&, const Track&, const ino_t, const std::shared_ptr<Song>& s) -> void
    {
      if (s->metadata.track <= song->metadata.track)
        return;

      auto next = audio.registerTrack(s);
      audio.addToPlaylist(next);
    });

  mpris.updateMetadata();
}

static auto jsonEscape(const std::string& s) -> std::string
{
  std::string out;
  out.reserve(s.size());

  for (char c : s)
  {
    switch (c)
    {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out += c;
    }
  }
  return out;
}

static auto unwrapSingleStringArg(const std::string& req) -> std::string
{
  std::string s = req;

  // Trim whitespace (optional but safe)
  auto is_space = [](char c) -> int { return std::isspace(static_cast<unsigned char>(c)); };
  while (!s.empty() && is_space(s.front()))
    s.erase(s.begin());
  while (!s.empty() && is_space(s.back()))
    s.pop_back();

  // Expect: ["..."]
  if (s.size() >= 4 && s.front() == '[' && s.back() == ']')
  {
    s = s.substr(1, s.size() - 2); // remove [ ]

    // Now expect: "..."
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
    {
      s = s.substr(1, s.size() - 2); // remove quotes
    }
  }

  return s;
}

void Interface::statusLoop(audio::Service& audio)
{
  while (m_isRunning.load())
  {
    // helpers::telemetry::updateTelemetryProgress(audio, m_currentPlay, m_lastPlayTick);

    if (m_cfgWatcher.pollChanged())
    {
      LOG_DEBUG("Configuration file changed, reloading...");
      loadConfig();
    }

    const auto pos = audio.getCurrentTrackInfo()->positionSec;
    const auto len = audio.getCurrentTrackInfo()->lengthSec;

    if (pos >= len)
    {
      // helpers::telemetry::endPlayback(audio, m_telemetryCtx, m_currentPlay, m_lastPlayTick);
      audio.nextTrack();
      // helpers::telemetry::beginPlayback(audio, m_telemetryCtx, m_currentPlay, m_lastPlayTick);
      m_mpris->updateMetadata();
      m_mpris->notify();
    }

    if (autoNextIfFinished(audio, *m_mpris, m_lastAutoNextTid, m_autoNextInProgress))
    {
      // helpers::telemetry::endPlayback(audio, m_telemetryCtx, m_currentPlay, m_lastPlayTick);
      // helpers::telemetry::beginPlayback(audio, m_telemetryCtx, m_currentPlay, m_lastPlayTick);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void Interface::loadConfig()
{
  // try
  // {
  //   tomlparser::Config::load();
  //
  //   config::colors::ConfigLoader   colorsCfg(FRONTEND_NAME);
  //   config::keybinds::ConfigLoader keysCfg(FRONTEND_NAME);
  //
  //   CmdlineConfig next;
  //   next.kb     = Keybinds::load(FRONTEND_NAME);
  //   next.colors = UiColors::load(FRONTEND_NAME);
  //
  //   loadMiscConfig(next.misc);
  //
  //   m_cfg.set(std::move(next));
  //
  //   LOG_INFO("Configuration loaded for {}'s keybinds and colors.", FRONTEND_NAME);
  // }
  // catch (const std::exception& e)
  // {
  //   LOG_ERROR("Interface::loadConfig failed: {}", e.what());
  // }
}

void Interface::run(audio::Service& audio)
{
  loadConfig();

  query::songmap::read::forEachSongInAlbum(
    *m_songMap, audio.getCurrentMetadata()->artist, audio.getCurrentMetadata()->album,
    [&](const Disc&, const Track&, const ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (song->metadata.track <= audio.getCurrentMetadata()->track)
        return;

      auto h = audio.registerTrack(song);
      audio.addToPlaylist(h);
    });

  query::songmap::read::forEachArtist(*m_songMap, [&](const Artist& artist, const AlbumMap&) -> void
                                      { m_artists.push_back(artist); });

  auto html = std::filesystem::absolute("src/frontend/webview/ui/index.html");

  m_web.bind(
    "__getAlbumsForArtist",
    [&](const std::string& id, const std::string& req, void*) -> void
    {
      const std::string artist = unwrapSingleStringArg(req);

      LOG_TRACE("Requested Artist: {}", artist);

      std::string json       = "[";
      bool        firstAlbum = true;

      query::songmap::read::forEachAlbum(
        *m_songMap,
        [&](const Artist& a, const Album& album, const DiscMap& discs) -> void
        {
          if (!utils::string::isEquals(a, artist))
            return;

          if (!firstAlbum)
            json += ",";
          firstAlbum = false;

          json += "{";
          json += R"("album":")" + jsonEscape(album) + "\",";
          json += "\"discs\":[";

          bool firstDisc = true;
          for (const auto& [disc, tracks] : discs)
          {
            if (!firstDisc)
              json += ",";
            firstDisc = false;

            json += "{";
            json += "\"disc\":" + std::to_string(disc) + ",";
            json += "\"tracks\":[";

            bool firstTrack = true;
            for (const auto& [_, inodeMap] : tracks)
            {
              for (const auto& [__, song] : inodeMap)
              {
                if (!firstTrack)
                  json += ",";
                firstTrack = false;

                json += "{";
                json += "\"track\":" + std::to_string(song->metadata.track) + ",";
                json += R"("title":")" + jsonEscape(song->metadata.title) + "\",";
                json += "\"duration\":" + std::to_string(song->metadata.duration);
                json += "}";
              }
            }

            json += "]";
            json += "}";
          }

          json += "]";
          json += "}";
        });

      json += "]";

      m_web.resolve(id, 0, json.c_str());
    },
    nullptr);

  m_web.bind(
    "__getArtists",
    [&](const std::string& id, const std::string&, void*) -> void
    {
      std::string json;
      json.reserve(m_artists.size() * 32);
      json += "[";

      bool first = true;
      for (const auto& artist : m_artists)
      {
        if (!first)
          json += ",";
        first = false;

        json += "\"";
        json += jsonEscape(artist);
        json += "\"";
      }

      json += "]";
      m_web.resolve(id, 0, json.c_str());
    },
    nullptr);

  m_web.bind(
    "__playTrack",
    [&](const std::string& id, const std::string& req, void*) -> void
    {
      // req arrives as ["Title"] → unwrap
      std::string title = unwrapSingleStringArg(req);

      auto song = query::songmap::read::findSongObjByTitle(*m_songMap, title);

      if (!song)
      {
        LOG_ERROR("Track not found: {}", title);
        m_web.resolve(id, 1, "Track not found");
        return;
      }

      playSongWithAlbumQueue(audio, *m_songMap, song, *m_mpris);

      m_web.resolve(id, 0, "ok");
    },
    nullptr);

  m_web.bind(
    "__quitApp",
    [&](const std::string& id, const std::string&, void*) -> void
    {
      LOG_INFO("Quit requested from Webview.");

      m_isRunning.store(false, std::memory_order_release);

      m_web.resolve(id, 0, "ok");
    },
    nullptr);

  m_web.bind(
    "__fetchMetadata",
    [&](const std::string& id, const std::string&, void*) -> void
    {
      std::string json;
      json.reserve(m_artists.size() * 32);
      json += "{";

      auto metOpt = audio.getCurrentMetadata();
      if (!metOpt)
      {
        LOG_ERROR("No audio metadata found!");
        return;
      }
      json += R"("title":")" + jsonEscape(metOpt->title) + "\",";
      json += R"("album":")" + jsonEscape(metOpt->album) + "\",";
      json += R"("genre":")" + jsonEscape(metOpt->genre) + "\",";
      json += R"("artUrl":")" + jsonEscape(metOpt->artUrl) + "\"";

      json += "}";
      m_web.resolve(id, 0, json.c_str());
    },
    nullptr);

  m_web.bind(
    "__playPauseTrack",
    [&](const std::string& id, const std::string&, void*) -> void
    {
      audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();

      m_web.resolve(id, 0, "ok");
    },
    nullptr);

  m_web.bind(
    "__prevTrack",
    [&](const std::string& id, const std::string&, void*) -> void
    {
      audio.previousTrack();
      m_web.resolve(id, 0, "ok");
    },
    nullptr);

  m_web.bind(
    "__nextTrack",
    [&](const std::string& id, const std::string&, void*) -> void
    {
      audio.nextTrack();
      m_web.resolve(id, 1, R"({"success":"next track"})");
    },
    nullptr);

  m_mpris->updateMetadata();
  m_mpris->notify();
  m_isRunning.store(true, std::memory_order_release);
  m_web.navigate(utils::fs::toRelFilePathUri(html).c_str());
  m_web.run();
}

} // namespace frontend::webview
