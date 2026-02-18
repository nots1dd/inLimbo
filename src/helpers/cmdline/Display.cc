#include "helpers/cmdline/Display.hpp"
#include "Logger.hpp"
#include "build/generated/PluginList.ipp"
#include "helpers/fs/LRC.hpp"
#include "lrc/Client.hpp"
#include "query/SongMap.hpp"
#include "telemetry/Analysis.hpp"
#include "toml/Parser.hpp"
#include "utils/string/Equals.hpp"
#include "utils/timer/Timer.hpp"

#include <iostream>
#include <map>
#include <set>

namespace helpers::cmdline
{

// ------------------------------------------------------------
// Print all frontend plugins built
// ------------------------------------------------------------
void printFrontendPlugins()
{
  std::cout << "\nAvailable frontends:\n";
  std::cout << "────────────────────────────\n";
  for (auto i : INLIMBO_FRONTEND_NAMES)
    std::cout << ">> " << i << "\n";
}

// ------------------------------------------------------------
// Print all audio devices found by registry (comptime)
// ------------------------------------------------------------
void printAudioBackends(const audio::BackendList& backends)
{
  std::cout << "\nAvailable Audio Backends:\n";
  std::cout << "────────────────────────────\n";

  for (const auto& b : backends)
  {
    std::cout << ">> " << b.name << ":\n";
    std::cout << "  - Description : " << b.description << "\n";
    std::cout << "  - Available   : " << (b.available ? "Yes" : "No") << "\n";
  }
}

// ------------------------------------------------------------
// Print all audio devices found by currently used audio backend
// ------------------------------------------------------------
void printAudioDevices(audio::Devices& devices)
{
  std::cout << "\nAvailable Audio Devices:\n";
  std::cout << "────────────────────────────\n";
  for (const auto& i : devices)
  {
    std::cout << ">> " << i.name.c_str() << ":\n";
    std::cout << "  - Description : " << i.description.c_str() << "\n";
    std::cout << "  - DeviceIndex : " << i.deviceIndex << "\n";
    std::cout << "  - IsDefault   : " << (i.isDefault ? "Yes" : "No") << "\n";
  }
}

// ------------------------------------------------------------
// Print all artists
// ------------------------------------------------------------
void printArtists(const threads::SafeMap<SongMap>& safeMap)
{
  std::cout << "\nArtists:\n";
  std::cout << "────────────────────────────\n";

  query::songmap::read::forEachArtist(safeMap, [&](const Artist& artist, const AlbumMap&) -> void
                                      { std::cout << "• " << artist << "\n"; });
}

// ------------------------------------------------------------
// Print song info
// ------------------------------------------------------------
void printSongInfoByTitle(const threads::SafeMap<SongMap>& safeMap,
                          const std::optional<Title>&      songName)
{
  if (!songName || songName->empty())
  {
    LOG_WARN("Song not found! (Title seems to be empty)");
    return;
  }

  auto songs = query::songmap::read::findAllSongsByTitle(safeMap, *songName);

  for (const auto& song : songs)
  {

    if (!song)
    {
      LOG_ERROR("Song not found: '{}' (MAYBE USE FUZZY SEARCH?)", *songName);
      return;
    }

    std::cout << "Song Information\n";
    std::cout << "────────────────────────────\n";

    std::cout << "INODE " << song->inode << ":\n\n";
    std::cout << "Title       : " << song->metadata.title << "\n";
    std::cout << "Artist      : " << song->metadata.artist << "\n";
    std::cout << "Album       : " << song->metadata.album << "\n";
    std::cout << "Genre       : " << song->metadata.genre << "\n";
    std::cout << "Duration    : " << song->metadata.duration << "s\n";
    std::cout << "Bitrate     : " << song->metadata.bitrate << "kbps\n";
    std::cout << "HasLyrics   : " << (song->metadata.lyrics.empty() ? "NO" : "YES") << "\n";
    std::cout << "HasArt      : " << (song->metadata.artUrl.empty() ? "NO" : "YES") << "\n";

    if (song->metadata.track > 0)
      std::cout << "Track       : " << song->metadata.track << "\n";

    if (song->metadata.trackTotal > 0)
      std::cout << "Track Total : " << song->metadata.trackTotal << "\n";

    if (song->metadata.discNumber > 0)
      std::cout << "Disc        : " << song->metadata.discNumber << "\n";

    if (song->metadata.discTotal > 0)
      std::cout << "Disc Total  : " << song->metadata.discTotal << "\n";

    if (song->metadata.year > 0)
      std::cout << "Year        : " << song->metadata.year << "\n";

    if (!song->metadata.filePath.empty())
      std::cout << "File Path   : " << song->metadata.filePath.c_str() << "\n";

    if (!song->metadata.artUrl.empty())
      std::cout << "Album Art   : " << song->metadata.artUrl.c_str() << "\n";

    if (!song->metadata.comment.empty())
      std::cout << "Comment     : " << song->metadata.comment << "\n";

    std::cout << "\n";
  }
}

// ------------------------------------------------------------
// Print song lyrics
// ------------------------------------------------------------
void printSongLyrics(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle)
{
  if (songTitle.empty())
  {
    LOG_WARN("Song not found! (Title seems to be empty.)");
    return;
  }

  auto song = query::songmap::read::findSongObjByTitle(safeMap, songTitle);

  if (!song)
  {
    LOG_ERROR("Song not found: '{}' (MAYBE USE FUZZY SEARCH?)", songTitle);
    return;
  }

  std::cout << "\nLyrics for '" << song->metadata.title << "' by " << song->metadata.artist
            << ":\n";
  std::cout << "────────────────────────────\n";

  if (!song->metadata.lyrics.empty())
  {
    std::cout << song->metadata.lyrics << "\n";
    return;
  }

  auto cachePath =
    helpers::lrc::genLRCFilePath(song->metadata.artist, song->metadata.title, song->metadata.album);

  if (auto cached = helpers::lrc::tryReadCachedLRC(cachePath))
  {
    LOG_INFO("Loaded lyrics from cache: {}", cachePath.string());
    std::cout << *cached << "\n";
    return;
  }

  LOG_WARN("No lyrics in metadata or cache - fetching from LRCLIB...");

  ::lrc::Client client;

  ::lrc::Query query;
  query.artist = song->metadata.artist;
  query.album  = song->metadata.album;
  query.track  = song->metadata.title;

  auto res = client.fetchBestMatchAndCache(query);

  if (!res.ok())
  {
    LOG_ERROR("lrc::Client error: {}", res.error.message);
    return;
  }

  auto& [lyrics, path] = res.value;

  LOG_INFO("Fetched + cached lyrics at '{}'", path);

  if (lyrics.plainLyrics)
    std::cout << *lyrics.plainLyrics << "\n";
}

void printAlbums(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist)
{
  std::cout << (!artist.has_value() ? "\nAll Albums:\n" : "\nAlbums by " + *artist + ":\n");
  std::cout << "────────────────────────────\n";

  query::songmap::read::forEachAlbum(
    safeMap,
    [&](const Artist& a, const Album& album, const DiscMap&) -> void
    {
      if (artist && !artist->empty() && !utils::string::isEquals(a, *artist))
        return;
      std::cout << "• " << album << "\n";
    });
}

// ------------------------------------------------------------
// Print song tree (optionally filtered by artist)
// ------------------------------------------------------------
void printSongTree(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist)
{
  using TInfo = std::pair<Track, Title>;
  using DMap  = std::map<Disc, std::vector<TInfo>>;
  using AMap  = std::map<Album, DMap>;
  using Tree  = std::map<Artist, AMap>;

  Tree tree;

  query::songmap::read::forEachSong(
    safeMap,
    [&](const Artist& a, const Album& al, const Disc disc, const Track track, const ino_t,
        const std::shared_ptr<Song>& song) -> void
    {
      if (artist && !artist->empty() && !utils::string::isEquals(a, *artist))
        return;

      tree[a][al][disc].emplace_back(track, song->metadata.title);
    });

  std::cout << "\nSong tree:\n";
  std::cout << "────────────────────────────\n";

  for (const auto& [a, albums] : tree)
  {
    std::cout << a << "\n";

    for (const auto& [al, discs] : albums)
    {
      std::cout << "  ├─ " << al << "\n";

      for (const auto& [disc, tracks] : discs)
      {
        std::cout << "  │  ├─ Disc " << disc << "\n";

        for (const auto& [track, title] : tracks)
        {
          std::cout << "  │  │  └─ " << std::setw(2) << std::setfill('0') << track << " – " << title
                    << "\n";
        }
      }
    }
  }
}

// ------------------------------------------------------------
// Print genres
// ------------------------------------------------------------
void printGenres(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist)
{
  const bool hasArtist = artist && !artist->empty();

  std::cout << (!hasArtist ? "\nAll Genres:\n" : "\nGenres in songs by " + *artist + ":\n");

  std::cout << "────────────────────────────\n";

  if (hasArtist)
  {
    query::songmap::read::forEachGenreInArtist(safeMap, *artist,
                                               [&](const Genre& genre) -> void
                                               {
                                                 if (!genre.empty())
                                                   std::cout << "• " << genre << "\n";
                                               });
  }
  else
  {
    query::songmap::read::forEachGenre(safeMap,
                                       [&](const Genre& genre) -> void
                                       {
                                         if (!genre.empty())
                                           std::cout << "• " << genre << "\n";
                                       });
  }
}

// ------------------------------------------------------------
// Print songs by artist
// ------------------------------------------------------------
void printSongsByArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artist)
{
  if (artist.empty())
    return;

  std::cout << "\nSongs by " << artist << ":\n";
  std::cout << "────────────────────────────\n";

  query::songmap::read::forEachSongInArtist(
    safeMap, artist,
    [&](const Album&                album, const Disc, const Track, const ino_t,
        const std::shared_ptr<Song> song) -> void
    { std::cout << "• " << song->metadata.title << " [" << album << "]\n"; });
}

// ------------------------------------------------------------
// Print songs by album
// ------------------------------------------------------------
void printSongsByAlbum(const threads::SafeMap<SongMap>& safeMap, const Album& album)
{
  if (album.empty())
    return;

  std::cout << "\nAlbum '" << album << "':\n";
  std::cout << "────────────────────────────\n";

  Disc currentDisc = -1;

  query::songmap::read::forEachSong(
    safeMap,
    [&](const Artist&, const Album& al, const Disc, const Track, const ino_t,
        const std::shared_ptr<Song> song) -> void
    {
      if (!utils::string::isEquals(al, album))
        return;

      const Disc disc = song->metadata.discNumber > 0 ? song->metadata.discNumber : 1;

      if (disc != currentDisc)
      {
        currentDisc = disc;
        std::cout << "Disc " << currentDisc << ":\n";
      }

      std::cout << "  └─ " << song->metadata.track << ". " << song->metadata.title << "\n";
    });
}

// ------------------------------------------------------------
// Print songs by genre
// ------------------------------------------------------------
void printSongsByGenre(const threads::SafeMap<SongMap>& safeMap, const Genre& genre)
{
  if (genre.empty())
    return;

  std::cout << "\nSongs in genre '" << genre << "':\n";
  std::cout << "────────────────────────────\n";

  query::songmap::read::forEachSongInGenre(
    safeMap, genre,
    [&](const Artist& artist, const Album& album, const Disc, const Track, const ino_t,
        const std::shared_ptr<Song> song) -> void
    { std::cout << "• " << song->metadata.title << " — " << artist << " [" << album << "]\n"; });
}

// ------------------------------------------------------------
// Print song paths
// ------------------------------------------------------------
void printSongPaths(const threads::SafeMap<SongMap>& safeMap)
{
  std::cout << "\nSong Paths:\n";
  std::cout << "────────────────────────────\n";

  query::songmap::read::forEachSong(safeMap,
                                    [&](const Artist& artist, const Album&, const Disc, const Track,
                                        const ino_t, const std::shared_ptr<Song> song) -> void
                                    {
                                      std::cout
                                        << "• " << song->metadata.title << " — " << artist << "\n"
                                        << "    " << song->metadata.filePath.c_str() << "\n";
                                    });
}

// ------------------------------------------------------------
// Print library summary
// ------------------------------------------------------------
void printSummary(const threads::SafeMap<SongMap>& safeMap, const telemetry::Context& telemetryCtx)
{
  std::set<Artist> artists;
  std::set<Album>  albums;
  std::set<Genre>  genres;
  size_t           songs = 0;

  query::songmap::read::forEachSong(safeMap,
                                    [&](const Artist& artist, const Album& album, const Disc,
                                        const Track, const ino_t,
                                        const std::shared_ptr<Song> song) -> void
                                    {
                                      ++songs;
                                      artists.insert(artist);
                                      albums.insert(album);
                                      if (!song->metadata.genre.empty())
                                        genres.insert(song->metadata.genre);
                                    });

  std::cout << "\nLibrary Summary\n"
            << "────────────────────────────\n"
            << "Artists Count      : " << artists.size() << "\n"
            << "Albums Count       : " << albums.size() << "\n"
            << "Songs Count        : " << songs << "\n"
            << "Genres Count       : " << genres.size() << "\n\n"
            << "Library Name       : " << tomlparser::Config::getString("library", "name") << "\n"
            << "Directory          : " << tomlparser::Config::getString("library", "directory")
            << "\n\n";

  if (!telemetryCtx.isRegistryLoaded)
    return;

  bool printed = false;

  auto resolveSong =
    [&](telemetry::SongID id) -> std::optional<std::pair<std::string_view, std::string_view>>
  {
    if (id == telemetry::INVALID_TELEMETRY_ID)
      return std::nullopt;

    auto title  = telemetryCtx.registry.titles.toString(id);
    auto artist = telemetryCtx.registry.artists.toString(telemetryCtx.registry.songToArtist(id));

    if (!title || !artist)
      return std::nullopt;

    return std::pair{*title, *artist};
  };

  auto printSongLine = [&](const char* label, telemetry::SongID id) -> void
  {
    if (auto r = resolveSong(id))
    {
      std::cout << label << " : " << r->first << " by " << r->second << "\n";
      printed = true;
    }
  };

  const auto now = utils::timer::nowUnix();

  printSongLine("Most Played Song  ", telemetry::analysis::mostReplayedSong(telemetryCtx.store));
  printSongLine("Hottest Song      ", telemetry::analysis::hottestSong(telemetryCtx.store, now));
  printSongLine("First Listened    ", telemetry::analysis::firstListenedSong(telemetryCtx.store));
  printSongLine("Last Listened     ", telemetry::analysis::lastListenedSong(telemetryCtx.store));

  if (auto n = telemetryCtx.registry.artists.toString(
        telemetry::analysis::favoriteArtist(telemetryCtx.store)))
    std::cout << "Favorite Artist    : " << *n << "\n", printed = true;

  if (auto n = telemetryCtx.registry.albums.toString(
        telemetry::analysis::favoriteAlbum(telemetryCtx.store)))
    std::cout << "Favorite Album     : " << *n << "\n", printed = true;

  if (auto n = telemetryCtx.registry.genres.toString(
        telemetry::analysis::favoriteGenre(telemetryCtx.store)))
    std::cout << "Favorite Genre     : " << *n << "\n", printed = true;

  const auto totalListen = telemetry::analysis::totalListenTime(telemetryCtx.store);

  if (totalListen > 0.0)
    std::cout << "Total Listen Time  : " << utils::timer::fmtTime(totalListen) << "\n",
      printed = true;

  if (!printed)
    std::cout << "Not enough telemetry data\n";
}

} // namespace helpers::cmdline
