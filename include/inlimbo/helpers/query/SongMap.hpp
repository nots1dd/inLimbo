#pragma once

#include "Config.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "thread/Map.hpp"

#include <functional>

namespace helpers::query::songmap
{

namespace strhelp = utils::string;

namespace read
{

INLIMBO_API_CPP auto findSongByName(const threads::SafeMap<core::SongMap>& safeMap,
                                    const std::string& songName) -> const core::Song*;

INLIMBO_API_CPP auto findSongByNameAndArtist(const threads::SafeMap<core::SongMap>& safeMap,
                                             const Artist& artistName, const std::string& songName)
  -> const core::Song*;

INLIMBO_API_CPP auto getSongsByAlbum(const threads::SafeMap<core::SongMap>& safeMap,
                                     const Artist& artist, const Album& album) -> const core::Songs;

INLIMBO_API_CPP auto countTracks(const threads::SafeMap<core::SongMap>& safeMap) -> size_t;

INLIMBO_API_CPP void
forEachArtist(const threads::SafeMap<core::SongMap>&                           safeMap,
              const std::function<void(const Artist&, const core::AlbumMap&)>& fn);

INLIMBO_API_CPP void
forEachAlbum(const threads::SafeMap<core::SongMap>&                                        safeMap,
             const std::function<void(const Artist&, const Album&, const core::DiscMap&)>& fn);

INLIMBO_API_CPP void forEachDisc(
  const threads::SafeMap<core::SongMap>& safeMap,
  const std::function<void(const Artist&, const Album&, const Disc, const core::TrackMap&)>& fn);

INLIMBO_API_CPP void
forEachSong(const threads::SafeMap<core::SongMap>&                     safeMap,
            const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                     const ino_t, const core::Song&)>& fn);

} // namespace read

namespace mut
{

INLIMBO_API_CPP auto replaceSongObjAndUpdateMetadata(threads::SafeMap<core::SongMap>& safeMap,
                                                     const core::Song&                oldSong,
                                                     const core::Song&                newSong,
                                                     TagLibParser& parser) -> bool;

} // namespace mut

} // namespace helpers::query::songmap
