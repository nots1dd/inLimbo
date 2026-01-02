#include "core/SongTree.hpp"
#include "Logger.hpp"
#include <fstream>
#include <set>

namespace core
{

// ============================================================
// Song Implementations
// ============================================================

Song::Song(ino_t inode, Metadata metadata) : inode(inode), metadata(std::move(metadata)) {}

Song::Song() : inode(0), metadata() {}

// ============================================================
// SongTree Implementations
// ============================================================

void SongTree::addSong(const Song& song)
{
  auto& artistMap = m_songMap[song.metadata.artist];
  auto& albumMap  = artistMap[song.metadata.album];
  auto& discMap   = albumMap[song.metadata.discNumber];
  auto& trackMap  = discMap[song.metadata.track];

  // Insert song by inode
  trackMap[song.inode] = song;

  LOG_TRACE("Added song '{}' by '{}' [Album: {}, Disc: {}, Track: {}, inode: {}]",
            song.metadata.title, song.metadata.artist, song.metadata.album,
            song.metadata.discNumber, song.metadata.track, song.inode);
}

// ------------------------------------------------------------
void SongTree::saveToFile(const std::string& filename) const
{
  std::ofstream file(filename, std::ios::binary);
  if (!file)
    throw std::runtime_error("Failed to open file for saving.");

  cereal::BinaryOutputArchive archive(file);
  archive(*this);
}

// ------------------------------------------------------------
void SongTree::loadFromFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::binary);
  if (!file)
    throw std::runtime_error("Failed to open file for loading.");

  cereal::BinaryInputArchive archive(file);
  archive(*this);
}

// ------------------------------------------------------------
void SongTree::traverse(const SongTreeCallbacks& cb) const
{
  RECORD_FUNC_TO_BACKTRACE("SongTree::traverse");

  int             totalArtists = 0, totalAlbums = 0, totalDiscs = 0, totalSongs = 0;
  std::set<Genre> uniqueGenres;

  if (cb.onBegin)
    cb.onBegin();

  for (const auto& [artist, albums] : m_songMap)
  {
    ++totalArtists;
    if (cb.onArtist && cb.onArtist(artist) == VisitResult::Stop)
      goto done;

    for (const auto& [album, discs] : albums)
    {
      ++totalAlbums;
      if (cb.onAlbum && cb.onAlbum(artist, album) == VisitResult::Stop)
        goto done;

      for (const auto& [disc, tracks] : discs)
      {
        ++totalDiscs;
        if (cb.onDisc && cb.onDisc(artist, album, disc) == VisitResult::Stop)
          goto done;

        for (const auto& [track, inodes] : tracks)
        {
          for (const auto& [_, song] : inodes)
          {
            ++totalSongs;
            uniqueGenres.insert(song.metadata.genre);

            if (cb.onSong && cb.onSong(artist, album, disc, track, song) == VisitResult::Stop)
              goto done;
          }
        }
      }
    }
  }

done:
  if (cb.onSummary)
    cb.onSummary(totalArtists, totalAlbums, totalDiscs, totalSongs,
                 static_cast<int>(uniqueGenres.size()));

  if (cb.onEnd)
    cb.onEnd();
}

auto SongTree::findSong(const std::function<bool(const Song&)>& predicate,
                        const std::function<void(const Song&)>& onFound) const -> bool
{
  bool found = false;

  traverse({.onSong = [&](auto&, auto&, auto, auto, const Song& s) -> auto
            {
              if (predicate(s))
              {
                onFound(s);
                found = true;
                return VisitResult::Stop;
              }
              return VisitResult::Continue;
            }});

  return found;
}

void SongTree::forEach(const SongPredicate& pred, const SongTreeVisitor& visitor) const
{
  traverse({.onSong = [&](auto& a, auto& al, auto d, auto t, const Song& s) -> auto
            {
              if (!pred(a, al, d, t, s))
                return VisitResult::Continue;
              return visitor(a, al, d, t, s);
            }});
}

} // namespace core
