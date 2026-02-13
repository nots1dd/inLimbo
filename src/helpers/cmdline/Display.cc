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
  std::map<Artist, std::set<Album>> albums;

  query::songmap::read::forEachSong(safeMap,
                                    [&](const Artist& a, const Album& al, const Disc, const Track,
                                        const ino_t, const std::shared_ptr<Song>&) -> void
                                    {
                                      if (artist && !artist->empty() &&
                                          !utils::string::isEquals(a, *artist))
                                        return;

                                      albums[a].insert(al);
                                    });

  std::cout << "\nAlbums:\n";
  std::cout << "────────────────────────────\n";

  for (const auto& [a, als] : albums)
  {
    std::cout << a << "\n";
    for (const auto& al : als)
      std::cout << "  └─ " << al << "\n";
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
  // we could use aggregate queries here but since we need to iterate all songs for the summary
  // anyway, we might as well do it in one pass
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

  std::cout << "\nLibrary Summary\n";
  std::cout << "────────────────────────────\n";
  std::cout << "Artists Count      : " << artists.size() << "\n";
  std::cout << "Albums Count       : " << albums.size() << "\n";
  std::cout << "Songs Count        : " << songs << "\n";
  std::cout << "Genres Count       : " << genres.size() << "\n\n";

  std::cout << "Library Name       : " << tomlparser::Config::getString("library", "name") << "\n";
  std::cout << "Directory          : " << tomlparser::Config::getString("library", "directory")
            << "\n\n";

  if (telemetryCtx.isRegistryLoaded)
  {
    const auto mostPlayedSongId = telemetry::analysis::mostReplayedSong(telemetryCtx.store);

    const auto hottestSongId =
      telemetry::analysis::hottestSong(telemetryCtx.store, utils::timer::nowUnix());

    const auto titleMP  = telemetryCtx.registry.titles.toString(mostPlayedSongId);
    const auto titleHS  = telemetryCtx.registry.titles.toString(hottestSongId);
    const auto artistMP = telemetryCtx.registry.artists.toString(mostPlayedSongId);
    const auto artistHS = telemetryCtx.registry.artists.toString(hottestSongId);

    std::cout << "Most Played Song   : " << (titleMP ? *titleMP : "<unknown>") << " by "
              << (artistMP ? *artistMP : "<unknown>") << "\n";

    std::cout << "Hottest Song       : " << (titleHS ? *titleHS : "<unknown>") << " by "
              << (artistHS ? *artistHS : "<unknown>") << "\n";

    // ---- favorite artist ----
    const auto favArtistId = telemetry::analysis::favoriteArtist(telemetryCtx.store);
    if (auto name = telemetryCtx.registry.artists.toString(favArtistId))
      std::cout << "Favorite Artist    : " << *name << "\n";

    // ---- favorite album ----
    const auto favAlbumId = telemetry::analysis::favoriteAlbum(telemetryCtx.store);
    if (auto name = telemetryCtx.registry.albums.toString(favAlbumId))
      std::cout << "Favorite Album     : " << *name << "\n";

    // ---- favorite genre ----
    const auto favGenreId = telemetry::analysis::favoriteGenre(telemetryCtx.store);
    if (auto name = telemetryCtx.registry.genres.toString(favGenreId))
      std::cout << "Favorite Genre     : " << *name << "\n";

    std::cout << "Total Listen Time  : "
              << utils::timer::fmtTime(telemetry::analysis::totalListenTime(telemetryCtx.store))
              << "\n";
  }
}

} // namespace helpers::cmdline
