#pragma once

#include "Config.hpp"
#include "core/taglib/Parser.hpp"
#include "thread/Map.hpp"

#include <functional>

namespace query::songmap
{

namespace strhelp = utils::string;

namespace read
{

INLIMBO_API_CPP auto findSongByTitle(const threads::SafeMap<SongMap>& safeMap,
                                     const Title&                     songTitle) -> const Song*;

INLIMBO_API_CPP auto findSongByTitleAndArtist(const threads::SafeMap<SongMap>& safeMap,
                                              const Artist& artistName, const Title& songTitle)
  -> const Song*;

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

INLIMBO_API_CPP void
forEachSong(const threads::SafeMap<SongMap>&                     safeMap,
            const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                     const ino_t, const Song&)>& fn);

} // namespace read

namespace mut
{

INLIMBO_API_CPP auto replaceSongObjAndUpdateMetadata(threads::SafeMap<SongMap>& safeMap,
                                                     const Song& oldSong, const Song& newSong,
                                                     core::TagLibParser& parser) -> bool;

} // namespace mut

} // namespace query::songmap
