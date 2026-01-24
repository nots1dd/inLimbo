#pragma once

#include "Config.hpp"
#include "taglib/Parser.hpp"
#include "thread/Map.hpp"

#include <functional>

namespace query::songmap
{

namespace strhelp = utils::string;

namespace read
{

INLIMBO_API_CPP auto findSongPathByInode(const threads::SafeMap<SongMap>& safeMap,
                                         const ino_t                      givenInode) -> PathStr;

INLIMBO_API_CPP auto findSongObjByTitle(const threads::SafeMap<SongMap>& safeMap,
                                        const Title&                     songTitle) -> const Song*;

INLIMBO_API_CPP auto findSongObjByTitleFuzzy(const threads::SafeMap<SongMap>& safeMap,
                                             const Title& songTitle, size_t maxDistance = 3)
  -> const Song*;

INLIMBO_API_CPP auto findArtistFuzzy(const threads::SafeMap<SongMap>& safeMap,
                                     const Artist& artistName, size_t maxDistance = 2) -> Artist;

INLIMBO_API_CPP auto findAlbumFuzzy(const threads::SafeMap<SongMap>& safeMap,
                                    const Album& albumName, size_t maxDistance = 2) -> Album;

INLIMBO_API_CPP auto findGenreFuzzy(const threads::SafeMap<SongMap>& safeMap,
                                    const Genre& genreName, size_t maxDistance = 2) -> Genre;

INLIMBO_API_CPP auto findSongByTitleAndArtist(const threads::SafeMap<SongMap>& safeMap,
                                              const Artist& artistName, const Title& songTitle)
  -> const Song*;

INLIMBO_API_CPP auto findSongByTitleAndArtistFuzzy(const threads::SafeMap<SongMap>& safeMap,
                                                   const Title& songTitle, const Artist& songArtist,
                                                   size_t maxDistance) -> const Song*;

INLIMBO_API_CPP auto getSongsByAlbum(const threads::SafeMap<SongMap>& safeMap, const Artist& artist,
                                     const Album& album) -> const Songs;

INLIMBO_API_CPP auto countTracks(const threads::SafeMap<SongMap>& safeMap) -> size_t;

INLIMBO_API_CPP void forEachArtist(const threads::SafeMap<SongMap>& safeMap,
                                   const std::function<void(const Artist&, const AlbumMap&)>& fn);

INLIMBO_API_CPP void forEachSongInArtist(
  const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
  const std::function<void(const Album&, const Disc, const Track, const ino_t, const Song&)>& fn);

INLIMBO_API_CPP void
forEachAlbum(const threads::SafeMap<SongMap>&                                        safeMap,
             const std::function<void(const Artist&, const Album&, const DiscMap&)>& fn);

INLIMBO_API_CPP void forEachSongInAlbum(
  const threads::SafeMap<SongMap>& safeMap, const Artist& artistName, const Album& albumName,
  const std::function<void(const Disc, const Track, const ino_t, const Song&)>& fn);

INLIMBO_API_CPP void forEachDisc(
  const threads::SafeMap<SongMap>&                                                     safeMap,
  const std::function<void(const Artist&, const Album&, const Disc, const TrackMap&)>& fn);

INLIMBO_API_CPP void
forEachSongInDisc(const threads::SafeMap<SongMap>& safeMap, const Artist& artistName,
                  const Album& albumName, Disc discNumber,
                  const std::function<void(const Track, const ino_t, const Song&)>& fn);

INLIMBO_API_CPP void forEachGenre(const threads::SafeMap<SongMap>&         safeMap,
                                  const std::function<void(const Genre&)>& fn);

INLIMBO_API_CPP void
forEachSongInGenre(const threads::SafeMap<SongMap>& safeMap, const Genre& genreName,
                   const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                            const ino_t, const Song&)>& fn);

INLIMBO_API_CPP void
forEachSong(const threads::SafeMap<SongMap>&                     safeMap,
            const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                     const ino_t, const Song&)>& fn);

} // namespace read

namespace mut
{

INLIMBO_API_CPP auto replaceSongObjAndUpdateMetadata(threads::SafeMap<SongMap>& safeMap,
                                                     const Song& oldSong, const Song& newSong,
                                                     taglib::Parser& parser) -> bool;

} // namespace mut

} // namespace query::songmap
