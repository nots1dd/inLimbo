#include "query/SongMap.hpp"
#include "InLimbo-Types.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"
#include "utils/algorithm/Levenshtein.hpp"
#include "utils/string/Equals.hpp"
#include <sys/types.h>

namespace query::songmap
{

namespace strhelp = utils::string;

namespace read
{

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

void forEachSong(const threads::SafeMap<SongMap>&                         safeMap,
                 const std::function<void(const Artist&, const Album&, Disc, Track, ino_t,
                                          const std::shared_ptr<Song>&)>& fn)
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

void forEachSongInArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                         const std::function<void(const Album&, const Disc, const Track,
                                                  const ino_t, const std::shared_ptr<Song>&)>& fn)
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
            for (auto& [inode, song] : inodeMap)
              fn(album, disc, track, inode, song);
    });
}

void forEachSongInAlbum(
  const threads::SafeMap<SongMap>& safeMap, const Artist& artistName, const Album& albumName,
  const std::function<void(const Disc, const Track, const ino_t, const std::shared_ptr<Song>&)>& fn)
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
          for (auto& [inode, song] : inodeMap)
            fn(disc, track, inode, song);
    });
}

void forEachSongInDisc(
  const threads::SafeMap<SongMap>& safeMap, const Artist& artistName, const Album& albumName,
  Disc                                                                               discNumber,
  const std::function<void(const Track, const ino_t, const std::shared_ptr<Song>&)>& fn)
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
        for (auto& [inode, song] : inodeMap)
          fn(track, inode, song);
    });
}

void forEachGenre(const threads::SafeMap<SongMap>&         safeMap,
                  const std::function<void(const Genre&)>& fn)
{
  std::set<Genre> genres;

  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              for (const auto& [inode, song] : inodeMap)
                genres.insert(song->metadata.genre);
    });

  for (const auto& g : genres)
    fn(g);
}

void forEachSongInGenre(
  const threads::SafeMap<SongMap>& safeMap, const Genre& genreName,
  const std::function<void(const Artist&, const Album&, const Disc, const Track, const ino_t,
                           const std::shared_ptr<Song>&)>& fn)
{
  safeMap.withReadLock(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            for (const auto& [track, inodeMap] : tracks)
              for (auto& [inode, song] : inodeMap)
                if (strhelp::isEquals(song->metadata.genre, genreName))
                  fn(artist, album, disc, track, inode, song);
    });
}

auto findSongPathByInode(const threads::SafeMap<SongMap>& safeMap, const ino_t givenInode)
  -> PathStr
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongPathByInode");

  PathStr path;

  forEachSong(safeMap,
              [&](const Artist&, const Album&, Disc, Track, ino_t inode,
                  const std::shared_ptr<Song>& song) -> void
              {
                if (inode == givenInode)
                  path = song->metadata.filePath;
              });

  return path;
}

auto findSongObjByTitle(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle)
  -> std::shared_ptr<Song>
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByName");

  std::shared_ptr<Song> result;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (!result && strhelp::isEquals(song->metadata.title, songTitle))
        result = song;
    });

  return result;
}

auto findSongObjByTitleFuzzy(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle,
                             size_t maxDistance) -> std::shared_ptr<Song>
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByTitleFuzzy");

  if (songTitle.empty())
    return {};

  const auto qLower = utils::string::transform::tolower_ascii(songTitle.c_str());

  std::shared_ptr<Song> bestSong;
  size_t                bestScore = SIZE_MAX;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (song->metadata.title.empty())
        return;

      const auto tLower = utils::string::transform::tolower_ascii(song->metadata.title.c_str());

      const size_t d = utils::algorithm::StringDistance::levenshteinDistance(
        tLower.c_str(), qLower.c_str(), maxDistance);

      if (d <= maxDistance && d < bestScore)
      {
        bestScore = d;
        bestSong  = song;
      }
    });

  return bestSong;
}

auto findArtistFuzzy(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                     size_t maxDistance) -> Artist
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findArtistFuzzy");

  if (artistName.empty())
    return {};

  const auto qLower = utils::string::transform::tolower_ascii(artistName.c_str());

  Artist bestArtist;
  size_t bestScore = SIZE_MAX;

  forEachArtist(safeMap,
                [&](const Artist& artist, const AlbumMap&) -> void
                {
                  if (artist.empty())
                    return;

                  const auto aLower = utils::string::transform::tolower_ascii(artist.c_str());

                  const size_t d = utils::algorithm::StringDistance::levenshteinDistance(
                    aLower.c_str(), qLower.c_str(), maxDistance);

                  if (d <= maxDistance && d < bestScore)
                  {
                    bestScore  = d;
                    bestArtist = artist;
                  }
                });

  return bestArtist;
}

auto findAlbumFuzzy(const threads::SafeMap<SongMap>& safeMap, const Album& albumName,
                    size_t maxDistance) -> Album
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findAlbumFuzzy");

  if (albumName.empty())
    return {};

  const auto qLower = utils::string::transform::tolower_ascii(albumName.c_str());

  Album  bestAlbum;
  size_t bestScore = SIZE_MAX;

  forEachArtist(safeMap,
                [&](const Artist&, const AlbumMap& albums) -> void
                {
                  for (const auto& [album, discs] : albums)
                  {
                    if (album.empty())
                      continue;

                    const auto alLower = utils::string::transform::tolower_ascii(album.c_str());

                    const size_t d = utils::algorithm::StringDistance::levenshteinDistance(
                      alLower.c_str(), qLower.c_str(), maxDistance);

                    if (d <= maxDistance && d < bestScore)
                    {
                      bestScore = d;
                      bestAlbum = album;
                    }
                  }
                });

  return bestAlbum;
}

auto findGenreFuzzy(const threads::SafeMap<SongMap>& safeMap, const Genre& genreName,
                    size_t maxDistance) -> Genre
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findGenreFuzzy");

  if (genreName.empty())
    return {};

  const auto qLower = utils::string::transform::tolower_ascii(genreName.c_str());

  Genre  bestGenre;
  size_t bestScore = SIZE_MAX;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      const auto& g = song->metadata.genre;
      if (g.empty())
        return;

      const auto gLower = utils::string::transform::tolower_ascii(g.c_str());

      const size_t d = utils::algorithm::StringDistance::levenshteinDistance(
        gLower.c_str(), qLower.c_str(), maxDistance);

      if (d <= maxDistance && d < bestScore)
      {
        bestScore = d;
        bestGenre = g;
      }
    });

  return bestGenre;
}

auto findSongByTitleAndArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                              const Title& songTitle) -> std::shared_ptr<Song>
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByNameAndArtist");

  std::shared_ptr<Song> result;

  forEachSong(safeMap,
              [&](const Artist&                artist, const Album&, Disc, Track, ino_t,
                  const std::shared_ptr<Song>& song) -> void
              {
                if (!result && strhelp::isEquals(artist, artistName) &&
                    strhelp::isEquals(song->metadata.title, songTitle))
                {
                  result = song;
                }
              });

  return result;
}

auto findSongByTitleAndArtistFuzzy(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle,
                                   const Artist& songArtist, size_t maxDistance)
  -> std::shared_ptr<Song>
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByTitleAndArtistFuzzy");

  if (songTitle.empty() || songArtist.empty())
    return {};

  std::shared_ptr<Song> bestSong;
  size_t                bestScoreSong = SIZE_MAX;

  forEachSong(safeMap,
              [&](const Artist&                artist, const Album&, Disc, Track, ino_t,
                  const std::shared_ptr<Song>& song) -> void
              {
                if (song->metadata.title.empty() || song->metadata.artist.empty())
                  return;

                const size_t sd = utils::algorithm::StringDistance::levenshteinDistance(
                  song->metadata.title, songTitle, maxDistance);

                const size_t ad = utils::algorithm::StringDistance::levenshteinDistance(
                  artist, songArtist, maxDistance);

                if (sd <= maxDistance && ad <= maxDistance && sd < bestScoreSong)
                {
                  bestScoreSong = sd;
                  bestSong      = song;
                }
              });

  return bestSong;
}

auto getSongsByAlbum(const threads::SafeMap<SongMap>& safeMap, const Artist& artist,
                     const Album& album) -> const Songs
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::getSongsByAlbum");

  Songs songs;

  forEachSong(safeMap,
              [&](const Artist& a, const Album& al, Disc, Track, ino_t,
                  const std::shared_ptr<Song>& song) -> void
              {
                if (strhelp::isEquals(a, artist) && strhelp::isEquals(al, album))
                  songs.push_back(song);
              });

  return songs;
}

auto countTracks(const threads::SafeMap<SongMap>& safeMap) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countTracks");

  size_t total = 0;

  forEachSong(safeMap,
              [&](const Artist&, const Album&, Disc, Track, ino_t,
                  const std::shared_ptr<Song>&) -> void { ++total; });

  return total;
}

} // namespace read

namespace mut
{

void sortSongMap(threads::SafeMap<SongMap>& safeMap, const query::sort::RuntimeSortPlan& rtSortPlan)
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::mut::sortSongMap");

  safeMap.withWriteLock(
    [&](auto& map) -> void
    {
      LOG_DEBUG("query::songmap::mut::sortSongMap: Provided runtime sort plan: artist={}, "
                "album={}, track={}",
                static_cast<int>(rtSortPlan.artist), static_cast<int>(rtSortPlan.album),
                static_cast<int>(rtSortPlan.track));

      query::sort::applyRuntimeSortPlan(map, rtSortPlan);
    });
}

auto replaceSongObjAndUpdateMetadata(threads::SafeMap<SongMap>&   safeMap,
                                     const std::shared_ptr<Song>& oldSong,
                                     const std::shared_ptr<Song>& newSong, taglib::Parser& parser)
  -> bool
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::mut::replaceSongObjByInode");

  ASSERT_MSG(oldSong->inode == newSong->inode, "Inode details changed! Will not be writing...");

  return safeMap.withWriteLock(
    [&](auto& map) -> bool
    {
      for (auto& [artist, albums] : map)
        for (auto& [album, discs] : albums)
          for (auto& [disc, tracks] : discs)
            for (auto& [track, inodeMap] : tracks)
              for (auto& [inodeKey, song] : inodeMap)
                if (inodeKey == oldSong->inode)
                {
                  LOG_INFO("Match found → Artist='{}', Title='{}', Album='{}', Disc={}, Track={}, "
                           "Inode={}",
                           artist, song->metadata.title, album, disc, track, inodeKey);

                  song = std::make_shared<Song>(*newSong);

                  if (parser.modifyMetadata(newSong->metadata.filePath.c_str(), newSong->metadata))
                    return true;

                  return false;
                }

      LOG_WARN("Inode '{}' not found in SongMap.", oldSong->inode);
      return false;
    });
}

} // namespace mut

} // namespace query::songmap
