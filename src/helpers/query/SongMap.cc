#include "helpers/query/SongMap.hpp"
#include "Logger.hpp"

namespace helpers::query::songmap
{

namespace read
{

auto findSongByName(const threads::SafeMap<core::SongMap>& safeMap, const std::string& songName)
  -> const core::Song*
{
  RECORD_FUNC_TO_BACKTRACE("helpers::query::songmap::read::findSongByName");

  return safeMap.withReadLock(
    [&](const auto& map) -> const core::Song*
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              for (const auto& [inode, song] : inodeMap)
                if (strhelp::isEquals(song.metadata.title, songName))
                  return &song;
      return nullptr;
    });
}

auto findSongByNameAndArtist(const threads::SafeMap<core::SongMap>& safeMap,
                             const Artist& artistName, const std::string& songName)
  -> const core::Song*
{
  RECORD_FUNC_TO_BACKTRACE("helpers::query::songmap::read::findSongByNameAndArtist");

  return safeMap.withReadLock(
    [&](const auto& map) -> const core::Song*
    {
      for (const auto& [artist, albums] : map)
      {
        if (!strhelp::isEquals(artist, artistName))
          continue;

        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              for (const auto& [inode, song] : inodeMap)
                if (strhelp::isEquals(song.metadata.title, songName))
                  return &song;
      }
      return nullptr;
    });
}

auto getSongsByAlbum(const threads::SafeMap<core::SongMap>& safeMap, const Artist& artist,
                     const Album& album) -> const core::Songs
{
  RECORD_FUNC_TO_BACKTRACE("helpers::query::songmap::read::getSongsByAlbum");

  return safeMap.withReadLock(
    [&](const auto& map) -> const core::Songs
    {
      core::Songs songs;

      for (const auto& [a, albums] : map)
      {
        if (!strhelp::isEquals(a, artist))
          continue;

        for (const auto& [alb, discs] : albums)
        {
          if (!strhelp::isEquals(alb, album))
            continue;

          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              for (const auto& [inode, song] : inodeMap)
                songs.push_back(song);

          return songs;
        }
      }
      return songs;
    });
}

auto countTracks(const threads::SafeMap<core::SongMap>& safeMap) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("helpers::query::songmap::read::countTracks");

  return safeMap.withReadLock(
    [](const auto& map) -> size_t
    {
      size_t total = 0;
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              total += inodeMap.size();
      return total;
    });
}

void forEachArtist(const threads::SafeMap<core::SongMap>&                           safeMap,
                   const std::function<void(const Artist&, const core::AlbumMap&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        fn(artist, albums);
    });
}

void forEachAlbum(const threads::SafeMap<core::SongMap>& safeMap,
                  const std::function<void(const Artist&, const Album&, const core::DiscMap&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          fn(artist, album, discs);
    });
}

void forEachDisc(
  const threads::SafeMap<core::SongMap>& safeMap,
  const std::function<void(const Artist&, const Album&, const Disc, const core::TrackMap&)>& fn)
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

void forEachSong(const threads::SafeMap<core::SongMap>&                     safeMap,
                 const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                          const ino_t, const core::Song&)>& fn)
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

auto replaceSongObjAndUpdateMetadata(threads::SafeMap<core::SongMap>& safeMap,
                                     const core::Song& oldSong, const core::Song& newSong,
                                     TagLibParser& parser) -> bool
{
  RECORD_FUNC_TO_BACKTRACE("helpers::query::songmap::mut::replaceSongObjByInode");

  ASSERT_MSG(oldSong.inode == newSong.inode, "Inode details changed! Will not be writing...");
  LOG_INFO("Song map size: {}", safeMap.snapshot().size());

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
                  LOG_INFO("Match found → Artist='{}', Album='{}', Disc={}, Track={}, Inode={}",
                           artist, album, disc, track, inodeKey);

                  song = newSong;

                  if (parser.modifyMetadata(newSong.metadata.filePath, newSong.metadata))
                    return true;

                  return false;
                }

      LOG_WARN("Inode '{}' not found in SongMap.", oldSong.inode);
      return false;
    });
}

} // namespace mut

} // namespace helpers::query::songmap
