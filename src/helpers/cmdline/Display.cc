#include "helpers/cmdline/Display.hpp"
#include "Logger.hpp"
#include "build/generated/PluginList.ipp"
#include "query/SongMap.hpp"
#include "utils/string/Equals.hpp"

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

  const Song* song = query::songmap::read::findSongByTitle(safeMap, *songName);

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

  const Song* song = query::songmap::read::findSongByTitle(safeMap, songTitle);

  if (!song)
  {
    LOG_ERROR("Song not found: '{}' (MAYBE USE FUZZY SEARCH?)", songTitle);
    return;
  }

  std::cout << "\nLyrics for '" << song->metadata.title << "' by " << song->metadata.artist
            << ":\n";
  std::cout << "────────────────────────────\n";
  std::cout << song->metadata.lyrics << "\n";
}

// ------------------------------------------------------------
// Print albums (optionally filtered by artist)
// ------------------------------------------------------------
void printAlbums(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist)
{
  std::map<Artist, std::set<Album>> albums;

  query::songmap::read::forEachSong(
    safeMap,
    [&](const Artist& a, const Album& al, const Disc, const Track, const ino_t, const Song&) -> void
    {
      if (artist && !artist->empty() && !utils::string::isEquals(a, *artist))
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
void printGenres(const threads::SafeMap<SongMap>& safeMap)
{
  std::set<Genre> genres;

  query::songmap::read::forEachSong(
    safeMap,
    [&](const Artist&, const Album&, const Disc, const Track, const ino_t, const Song& song) -> void
    {
      if (!song.metadata.genre.empty())
        genres.insert(song.metadata.genre);
    });

  std::cout << "\nGenres:\n";
  std::cout << "────────────────────────────\n";
  for (const auto& g : genres)
    std::cout << "• " << g << "\n";
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
    [&](const Album& album, const Disc, const Track, const ino_t, const Song& song) -> void
    { std::cout << "• " << song.metadata.title << " [" << album << "]\n"; });
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

  query::songmap::read::forEachSong(safeMap,
                                    [&](const Artist&, const Album& al, const Disc, const Track,
                                        const ino_t, const Song&    song) -> void
                                    {
                                      if (!utils::string::isEquals(al, album))
                                        return;

                                      const Disc disc =
                                        song.metadata.discNumber > 0 ? song.metadata.discNumber : 1;

                                      if (disc != currentDisc)
                                      {
                                        currentDisc = disc;
                                        std::cout << "Disc " << currentDisc << ":\n";
                                      }

                                      std::cout << "  └─ " << song.metadata.track << ". "
                                                << song.metadata.title << "\n";
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
        const Song& song) -> void
    { std::cout << "• " << song.metadata.title << " — " << artist << " [" << album << "]\n"; });
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
                                        const ino_t, const Song& song) -> void
                                    {
                                      std::cout << "• " << song.metadata.title << " — " << artist
                                                << "\n"
                                                << "    " << song.metadata.filePath.c_str() << "\n";
                                    });
}

// ------------------------------------------------------------
// Print library summary
// ------------------------------------------------------------
void printSummary(const threads::SafeMap<SongMap>& safeMap)
{
  std::set<Artist> artists;
  std::set<Album>  albums;
  std::set<Genre>  genres;
  size_t           songs = 0;

  query::songmap::read::forEachSong(safeMap,
                                    [&](const Artist& artist, const Album& album, const Disc,
                                        const Track, const ino_t, const Song& song) -> void
                                    {
                                      ++songs;
                                      artists.insert(artist);
                                      albums.insert(album);
                                      if (!song.metadata.genre.empty())
                                        genres.insert(song.metadata.genre);
                                    });

  std::cout << "\nLibrary Summary\n";
  std::cout << "────────────────────────────\n";
  std::cout << "Artists : " << artists.size() << "\n";
  std::cout << "Albums  : " << albums.size() << "\n";
  std::cout << "Songs   : " << songs << "\n";
  std::cout << "Genres  : " << genres.size() << "\n";
}

} // namespace helpers::cmdline
