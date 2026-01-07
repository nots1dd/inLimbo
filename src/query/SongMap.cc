#include "query/SongMap.hpp"
#include "Logger.hpp"

namespace query::songmap
{

namespace read
{

auto findSongByTitle(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle)
  -> const Song*
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByName");

  const Song* result = nullptr;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, const Disc, const Track, const ino_t, const Song& song) -> void
    {
      if (!result && strhelp::isEquals(song.metadata.title, songTitle))
        result = &song;
    });

  return result;
}

auto findSongByNameAndArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                             const Title& songTitle) -> const Song*
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByNameAndArtist");

  const Song* result = nullptr;

  forEachSong(safeMap,
              [&](const Artist& artist, const Album&, const Disc, const Track, const ino_t,
                  const Song&   song) -> void
              {
                if (!result && strhelp::isEquals(artist, artistName) &&
                    strhelp::isEquals(song.metadata.title, songTitle))
                {
                  result = &song;
                }
              });

  return result;
}

auto getSongsByAlbum(const threads::SafeMap<SongMap>& safeMap, const Artist& artist,
                     const Album& album) -> const Songs
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::getSongsByAlbum");

  Songs songs;

  forEachSong(safeMap,
              [&](const Artist& a, const Album& al, const Disc, const Track, const ino_t,
                  const Song& song) -> void
              {
                if (strhelp::isEquals(a, artist) && strhelp::isEquals(al, album))
                {
                  songs.push_back(song);
                }
              });

  return songs;
}

auto countTracks(const threads::SafeMap<SongMap>& safeMap) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countTracks");

  size_t total = 0;

  forEachSong(safeMap,
              [&](const Artist&, const Album&, const Disc, const Track, const ino_t,
                  const Song&) -> void { ++total; });

  return total;
}

void forEachArtist(const threads::SafeMap<SongMap>&                           safeMap,
                   const std::function<void(const Artist&, const AlbumMap&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        fn(artist, albums);
    });
}

void forEachSongInArtist(
  const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
  const std::function<void(const Album&, const Disc, const Track, const ino_t, const Song&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      auto itArtist = map.find(artistName);
      if (itArtist == map.end())
        return;

      for (const auto& [album, discs] : itArtist->second)
        for (const auto& [disc, tracks] : discs)
          for (const auto& [track, inodeMap] : tracks)
            for (const auto& [inode, song] : inodeMap)
              fn(album, disc, track, inode, song);
    });
}

void forEachAlbum(const threads::SafeMap<SongMap>&                                        safeMap,
                  const std::function<void(const Artist&, const Album&, const DiscMap&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          fn(artist, album, discs);
    });
}

void forEachSongInAlbum(
  const threads::SafeMap<SongMap>& safeMap, const Artist& artistName, const Album& albumName,
  const std::function<void(const Disc, const Track, const ino_t, const Song&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      auto itArtist = map.find(artistName);
      if (itArtist == map.end())
        return;

      auto itAlbum = itArtist->second.find(albumName);
      if (itAlbum == itArtist->second.end())
        return;

      for (const auto& [disc, tracks] : itAlbum->second)
        for (const auto& [track, inodeMap] : tracks)
          for (const auto& [inode, song] : inodeMap)
            fn(disc, track, inode, song);
    });
}

void forEachDisc(
  const threads::SafeMap<SongMap>&                                                     safeMap,
  const std::function<void(const Artist&, const Album&, const Disc, const TrackMap&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            fn(artist, album, disc, tracks);
    });
}

void forEachSongInDisc(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                       const Album& albumName, Disc discNumber,
                       const std::function<void(const Track, const ino_t, const Song&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      auto itArtist = map.find(artistName);
      if (itArtist == map.end())
        return;

      auto itAlbum = itArtist->second.find(albumName);
      if (itAlbum == itArtist->second.end())
        return;

      auto itDisc = itAlbum->second.find(discNumber);
      if (itDisc == itAlbum->second.end())
        return;

      for (const auto& [track, inodeMap] : itDisc->second)
        for (const auto& [inode, song] : inodeMap)
          fn(track, inode, song);
    });
}

void forEachSong(const threads::SafeMap<SongMap>&                     safeMap,
                 const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                          const ino_t, const Song&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              for (const auto& [inode, song] : inodeMap)
                fn(artist, album, disc, track, inode, song);
    });
}

} // namespace read

namespace mut
{

auto replaceSongObjAndUpdateMetadata(threads::SafeMap<SongMap>& safeMap, const Song& oldSong,
                                     const Song& newSong, core::TagLibParser& parser) -> bool
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::mut::replaceSongObjByInode");

  ASSERT_MSG(oldSong.inode == newSong.inode, "Inode details changed! Will not be writing...");

  return safeMap.withWriteLock(
    [&](auto& map) -> bool
    {
      for (auto& [artist, albums] : map)
        for (auto& [album, discs] : albums)
          for (auto& [disc, tracks] : discs)
            for (auto& [track, inodeMap] : tracks)
              for (auto& [inodeKey, song] : inodeMap)
                if (inodeKey == oldSong.inode)
                {
                  LOG_INFO("Match found → Artist='{}', Title='{}', Album='{}', Disc={}, Track={}, "
                           "Inode={}",
                           artist, song.metadata.title, album, disc, track, inodeKey);

                  song = newSong;

                  if (parser.modifyMetadata(newSong.metadata.filePath.c_str(), newSong.metadata))
                    return true;

                  return false;
                }

      LOG_WARN("Inode '{}' not found in SongMap.", oldSong.inode);
      return false;
    });
}

} // namespace mut

} // namespace query::songmap
