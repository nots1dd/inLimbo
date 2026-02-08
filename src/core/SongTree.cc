#include "core/SongTree.hpp"
#include "Logger.hpp"
#include <fstream>

namespace core
{

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
  trackMap[song.inode] = std::make_shared<Song>(song);

  LOG_TRACE("Added song '{}' by '{}' [Album: {}, Disc: {}, Track: {}, inode: {}]",
            song.metadata.title, song.metadata.artist, song.metadata.album,
            song.metadata.discNumber, song.metadata.track, song.inode);
}

// ------------------------------------------------------------
void SongTree::saveToFile(const utils::string::SmallString& filename) const
{
  std::ofstream file(filename.c_str(), std::ios::binary);
  if (!file)
    throw std::runtime_error("SongTree::saveToFile: Failed to open file for saving.");

  cereal::BinaryOutputArchive archive(file);
  archive(*this);
}

// ------------------------------------------------------------
void SongTree::loadFromFile(const utils::string::SmallString& filename)
{
  std::ifstream file(filename.c_str(), std::ios::binary);
  if (!file)
    throw std::runtime_error("SongTree::loadFromFile: Failed to open file for loading.");

  cereal::BinaryInputArchive archive(file);
  archive(*this);
}

} // namespace core
