#include "query/SongMap.hpp"
#include "InLimbo-Types.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"
#include "utils/algorithm/Levenshtein.hpp"
#include "utils/string/Equals.hpp"
#include <sys/types.h>

#include <algorithm>

namespace query::songmap
{

namespace strhelp = utils::string;

namespace read
{

void forEachArtist(const threads::SafeMap<SongMap>&                           safeMap,
                   const std::function<void(const Artist&, const AlbumMap&)>& fn)
{
  safeMap.read(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        fn(artist, albums);
    });
}

void forEachAlbum(const threads::SafeMap<SongMap>&                                        safeMap,
                  const std::function<void(const Artist&, const Album&, const DiscMap&)>& fn)
{
  safeMap.read(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          fn(artist, album, discs);
    });
}

void forEachDisc(
  const threads::SafeMap<SongMap>&                                                     safeMap,
  const std::function<void(const Artist&, const Album&, const Disc, const TrackMap&)>& fn)
{
  safeMap.read(
    [&](const auto& map) -> void
    {
      for (const auto& [artist, albums] : map)
        for (const auto& [album, discs] : albums)
          for (const auto& [disc, tracks] : discs)
            fn(artist, album, disc, tracks);
    });
}

void forEachSong(const threads::SafeMap<SongMap>&                         safeMap,
                 const std::function<void(const Artist&, const Album&, Disc, Track, ino_t,
                                          const std::shared_ptr<Song>&)>& fn)
{
  safeMap.read(
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
  safeMap.read(
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
  safeMap.read(
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
  safeMap.read(
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

  safeMap.read(
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
  safeMap.read(
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

void forEachGenreInArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                          const std::function<void(const Genre&)>& fn)
{
  std::set<Genre> genres;

  safeMap.read(
    [&](const auto& map) -> void
    {
      auto itArtist = map.find(artistName);
      if (itArtist == map.end())
        return;

      for (const auto& [album, discs] : itArtist->second)
        for (const auto& [disc, tracks] : discs)
          for (const auto& [track, inodeMap] : tracks)
            for (const auto& [inode, song] : inodeMap)
            {
              if (!song->metadata.genre.empty())
                genres.insert(song->metadata.genre);
            }
    });

  for (const auto& g : genres)
    fn(g);
}

auto findAllSongsByTitle(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle) -> Songs
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findAllSongsByTitle");

  Songs results;

  if (songTitle.empty())
    return results;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (strhelp::isEquals(song->metadata.title, songTitle))
        results.push_back(song);
    });

  return results;
}

auto findAllSongsByTitleFuzzy(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle,
                              size_t maxDistance) -> Songs
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findAllSongsByTitleFuzzy");

  Songs results;

  if (songTitle.empty())
    return results;

  const auto qLower = utils::string::transform::tolower_ascii(songTitle.c_str());

  std::vector<std::pair<size_t, std::shared_ptr<Song>>> matches;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (song->metadata.title.empty())
        return;

      const auto tLower = utils::string::transform::tolower_ascii(song->metadata.title.c_str());

      const size_t d = utils::algorithm::StringDistance::levenshteinDistance(
        tLower.c_str(), qLower.c_str(), maxDistance);

      if (d <= maxDistance)
        matches.emplace_back(d, song);
    });

  std::ranges::sort(matches,
                    [](const auto& a, const auto& b) -> bool { return a.first < b.first; });

  results.reserve(matches.size());
  for (auto& [_, song] : matches)
    results.push_back(std::move(song));

  return results;
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

auto getSongsByArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artist) -> Songs
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::getSongsByArtist");

  Songs songs;

  if (artist.empty())
    return songs;

  forEachSong(safeMap,
              [&](const Artist&                a, const Album&, Disc, Track, ino_t,
                  const std::shared_ptr<Song>& song) -> void
              {
                if (strhelp::isEquals(a, artist))
                  songs.push_back(song);
              });

  return songs;
}

auto getSongsByGenre(const threads::SafeMap<SongMap>& safeMap, const Genre& genre) -> Songs
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::getSongsByGenre");

  Songs songs;

  if (genre.empty())
    return songs;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (strhelp::isEquals(song->metadata.genre, genre))
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

auto countSongsByArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artist) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countSongsByArtist");

  if (artist.empty())
    return 0;

  size_t count = 0;

  forEachSong(
    safeMap,
    [&](const Artist& a, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>&) -> void
    {
      if (strhelp::isEquals(a, artist))
        ++count;
    });

  return count;
}

auto countSongsByAlbum(const threads::SafeMap<SongMap>& safeMap, const Artist& artist,
                       const Album& album) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countSongsByAlbum");

  if (artist.empty() || album.empty())
    return 0;

  size_t count = 0;

  safeMap.read(
    [&](const auto& map) -> void
    {
      auto itArtist = map.find(artist);
      if (itArtist == map.end())
        return;

      auto itAlbum = itArtist->second.find(album);
      if (itAlbum == itArtist->second.end())
        return;

      for (const auto& [disc, tracks] : itAlbum->second)
        for (const auto& [track, inodeMap] : tracks)
          count += inodeMap.size();
    });

  return count;
}

auto countSongsByGenre(const threads::SafeMap<SongMap>& safeMap, const Genre& genre) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countSongsByGenre");

  if (genre.empty())
    return 0;

  size_t count = 0;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (strhelp::isEquals(song->metadata.genre, genre))
        ++count;
    });

  return count;
}

auto countArtists(const threads::SafeMap<SongMap>& safeMap) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countArtists");

  std::set<Artist> artists;

  forEachArtist(safeMap,
                [&](const Artist& artist, const AlbumMap&) -> void
                {
                  if (!artist.empty())
                    artists.insert(artist);
                });

  return artists.size();
}

auto countAlbums(const threads::SafeMap<SongMap>& safeMap) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countAlbums");

  std::set<Album> albums;

  forEachAlbum(safeMap,
               [&](const Artist&, const Album& album, const DiscMap&) -> void
               {
                 if (!album.empty())
                   albums.insert(album);
               });

  return albums.size();
}

auto countGenres(const threads::SafeMap<SongMap>& safeMap) -> size_t
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countGenres");

  std::set<Genre> genres;

  forEachSong(
    safeMap,
    [&](const Artist&, const Album&, Disc, Track, ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (!song->metadata.genre.empty())
        genres.insert(song->metadata.genre);
    });

  return genres.size();
}

} // namespace read

namespace mut
{

void sortSongMap(threads::SafeMap<SongMap>& safeMap, const query::sort::RuntimeSortPlan& rtSortPlan)
{
  RECORD_FUNC_TO_BACKTRACE("query::songmap::mut::sortSongMap");

  safeMap.update(
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

  return safeMap.update(
    [&](auto& map) -> bool
    {
      for (auto& [artist, albums] : map)
        for (auto& [album, discs] : albums)
          for (auto& [disc, tracks] : discs)
            for (auto& [track, inodeMap] : tracks)
              for (auto& [inodeKey, song] : inodeMap)
                if (inodeKey == oldSong->inode)
                {
                  LOG_INFO("Match found -> Artist='{}', Title='{}', Album='{}', Disc={}, Track={}, "
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
