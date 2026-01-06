#include "helpers/cmdline/Display.hpp"
#include "core/SongTreeIter.hpp"

#include <iomanip>
#include <iostream>
#include <map>
#include <set>

using namespace core;

namespace helpers::cmdline
{

// ------------------------------------------------------------
// Print all artists
// ------------------------------------------------------------
void printArtists(const SongTree& tree)
{
  std::set<Artist> artists;

  for (const Song& s : tree.range({}))
    artists.insert(s.metadata.artist);

  std::cout << "\nArtists:\n";
  std::cout << "────────────────────────────\n";
  for (const auto& a : artists)
    std::cout << "• " << a << "\n";
}

// ------------------------------------------------------------
// Print song info
// ------------------------------------------------------------
void printSongInfo(const SongTree& tree, const std::optional<Title>& songName)
{
  const auto pred = songName ? query::song::sort::byTitle(*songName) : query::song::SongPredicate{};

  bool printedHeader = false;
  bool found         = false;

  for (const Song& s : tree.range(pred))
  {
    if (!printedHeader)
    {
      std::cout << "Song Information\n";
      std::cout << "────────────────────────────\n";
      printedHeader = true;
    }

    found = true;

    std::cout << "INODE " << s.inode << ":\n\n";
    std::cout << "Title       : " << s.metadata.title << "\n";
    std::cout << "Artist      : " << s.metadata.artist << "\n";
    std::cout << "Album       : " << s.metadata.album << "\n";
    std::cout << "Genre       : " << s.metadata.genre << "\n";
    std::cout << "Duration    : " << s.metadata.duration << "s\n";
    std::cout << "Bitrate     : " << s.metadata.bitrate << "kbps\n";
    std::cout << "HasLyrics   : " << (s.metadata.lyrics.empty() ? "NO" : "YES") << "\n";
    std::cout << "HasArt      : " << (s.metadata.artUrl.empty() ? "NO" : "YES") << "\n";

    if (s.metadata.track > 0)
      std::cout << "Track       : " << s.metadata.track << "\n";

    if (s.metadata.trackTotal > 0)
      std::cout << "Track Total : " << s.metadata.trackTotal << "\n";

    if (s.metadata.discNumber > 0)
      std::cout << "Disc        : " << s.metadata.discNumber << "\n";

    if (s.metadata.discTotal > 0)
      std::cout << "Disc Total  : " << s.metadata.discTotal << "\n";

    if (s.metadata.year > 0)
      std::cout << "Year        : " << s.metadata.year << "\n";

    if (!s.metadata.filePath.empty())
      std::cout << "File Path   : " << s.metadata.filePath.c_str() << "\n";

    if (!s.metadata.comment.empty())
      std::cout << "Comment     : " << s.metadata.comment << "\n";

    std::cout << "\n";
  }

  if (!found)
  {
    std::cout << "\nSong not found";
    if (songName)
      std::cout << ": " << *songName;
    std::cout << "\n";
  }
}

// ------------------------------------------------------------
// Print albums (optionally filtered by artist)
// ------------------------------------------------------------
void printAlbums(const SongTree& tree, const std::optional<Artist>& artist)
{
  std::map<Artist, std::set<Album>> albums;

  const auto pred = artist ? query::song::sort::byArtist(*artist) : query::song::SongPredicate{};

  for (const Song& s : tree.range(pred))
    albums[s.metadata.artist].insert(s.metadata.album);

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
void printGenres(const SongTree& tree)
{
  std::set<Genre> genres;

  for (const Song& s : tree.range({}))
    genres.insert(s.metadata.genre);

  std::cout << "\nGenres:\n";
  std::cout << "────────────────────────────\n";
  for (const auto& g : genres)
    std::cout << "• " << g << "\n";
}

// ------------------------------------------------------------
// Print songs (optionally filtered)
// ------------------------------------------------------------
void printSongs(const SongTree& tree, query::song::SongPredicate pred)
{
  std::cout << "\nSongs:\n";
  std::cout << "────────────────────────────\n";

  for (const Song& s : tree.range(pred))
  {
    std::cout << "• " << s.metadata.title << " — " << s.metadata.artist << " [" << s.metadata.album
              << "]\n";
  }
}

void printSongsByArtist(const SongTree& tree, const Artist& artist)
{
  const auto pred = query::song::sort::byArtist(artist);

  std::cout << "\nSongs by " << artist << ":\n";
  std::cout << "────────────────────────────\n";

  for (const Song& s : tree.range(pred))
  {
    std::cout << "• " << s.metadata.title << " [" << s.metadata.album << "]\n";
  }
}

void printSongsByAlbum(const SongTree& tree, const Album& album)
{
  std::cout << "\nAlbum '" << album << "':\n";
  std::cout << "────────────────────────────\n";

  const auto pred = query::song::sort::byAlbum(album);

  Disc currentDisc = -1;

  for (const Song& s : tree.range(pred))
  {
    Disc disc = s.metadata.discNumber > 0 ? s.metadata.discNumber : 1;

    if (disc != currentDisc)
    {
      currentDisc = disc;
      std::cout << "Disc " << currentDisc << ":\n";
    }

    std::cout << "  └─ " << s.metadata.track << ". " << s.metadata.title << "\n";
  }
}

void printSongsByGenre(const SongTree& tree, const Genre& genre)
{
  const auto pred = query::song::sort::byGenre(genre);

  std::cout << "\nSongs in genre '" << genre << "':\n";
  std::cout << "────────────────────────────\n";

  for (const Song& s : tree.range(pred))
  {
    std::cout << "• " << s.metadata.title << " — " << s.metadata.artist << " [" << s.metadata.album
              << "]\n";
  }
}

// ------------------------------------------------------------
// Print song paths (title, artist, absolute path)
// ------------------------------------------------------------
void printSongPaths(const SongTree& tree, query::song::SongPredicate pred)
{
  std::cout << "\nSong Paths:\n";
  std::cout << "────────────────────────────\n";

  for (const Song& s : tree.range(pred))
  {
    std::cout << "• " << s.metadata.title << " — " << s.metadata.artist << "\n"
              << "    " << s.metadata.filePath.c_str() << "\n";
  }
}

// ------------------------------------------------------------
// Print library summary
// ------------------------------------------------------------
void printSummary(const SongTree& tree)
{
  std::set<Artist> artists;
  std::set<Album>  albums;
  std::set<Genre>  genres;
  size_t           songs = 0;

  for (const Song& s : tree.range({}))
  {
    ++songs;
    artists.insert(s.metadata.artist);
    albums.insert(s.metadata.album);
    genres.insert(s.metadata.genre);
  }

  std::cout << "\nLibrary Summary\n";
  std::cout << "────────────────────────────\n";
  std::cout << "Artists : " << artists.size() << "\n";
  std::cout << "Albums  : " << albums.size() << "\n";
  std::cout << "Songs   : " << songs << "\n";
  std::cout << "Genres  : " << genres.size() << "\n";
}

// ------------------------------------------------------------
// Print songs by artist + album
// ------------------------------------------------------------
void printArtistAlbum(const SongTree& tree, const Artist& artist, const Album& album)
{
  const auto pred = query::song::sort::byArtistAlbum(artist, album);

  std::cout << "\n" << artist << " — " << album << "\n";
  std::cout << "────────────────────────────\n";

  for (const Song& s : tree.range(pred))
  {
    std::cout << std::setw(2) << s.metadata.track << ". " << s.metadata.title << "\n";
  }
}

} // namespace helpers::cmdline
