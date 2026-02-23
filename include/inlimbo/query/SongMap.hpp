#pragma once

#include "Config.hpp"
#include "query/sort/Engine.hpp"
#include "taglib/Parser.hpp"

#include <functional>

namespace query::songmap
{

namespace read
{

// ==--------------------==
// Primitive iterations
// ==--------------------==

// Iterate all artists
INLIMBO_API_CPP void forEachArtist(const TS_SongMap& safeMap,
                                   const std::function<void(const Artist&, const AlbumMap&)>& fn);

// Iterate all albums
INLIMBO_API_CPP void
forEachAlbum(const TS_SongMap&                                                       safeMap,
             const std::function<void(const Artist&, const Album&, const DiscMap&)>& fn);

// Iterate all discs
INLIMBO_API_CPP void forEachDisc(
  const TS_SongMap&                                                                    safeMap,
  const std::function<void(const Artist&, const Album&, const Disc, const TrackMap&)>& fn);

// Iterate all songs
INLIMBO_API_CPP void
forEachSong(const TS_SongMap&                                        safeMap,
            const std::function<void(const Artist&, const Album&, Disc, Track, ino_t,
                                     const std::shared_ptr<Song>&)>& fn);

// Iterate songs inside artist
INLIMBO_API_CPP void
forEachSongInArtist(const TS_SongMap& safeMap, const Artist& artistName,
                    const std::function<void(const Album&, const Disc, const Track, const ino_t,
                                             const std::shared_ptr<Song>&)>& fn);

// Iterate songs inside album
INLIMBO_API_CPP void forEachSongInAlbum(
  const TS_SongMap& safeMap, const Artist& artistName, const Album& albumName,
  const std::function<void(const Disc, const Track, const ino_t, const std::shared_ptr<Song>&)>&
    fn);

// Iterate songs inside disc
INLIMBO_API_CPP void forEachSongInDisc(
  const TS_SongMap& safeMap, const Artist& artistName, const Album& albumName, Disc discNumber,
  const std::function<void(const Track, const ino_t, const std::shared_ptr<Song>&)>& fn);

// Iterate unique genres
INLIMBO_API_CPP void forEachGenre(const TS_SongMap&                        safeMap,
                                  const std::function<void(const Genre&)>& fn);

// Iterate songs inside genre
INLIMBO_API_CPP void
forEachSongInGenre(const TS_SongMap& safeMap, const Genre& genreName,
                   const std::function<void(const Artist&, const Album&, const Disc, const Track,
                                            const ino_t, const std::shared_ptr<Song>&)>& fn);

void forEachGenreInArtist(const TS_SongMap& safeMap, const Artist& artistName,
                          const std::function<void(const Genre&)>& fn);

// ==--------------------==
// Direct Lookups / Finders
// ==--------------------==

// Find all songs with exact title match (returns all matches across artists/albums)
INLIMBO_API_CPP auto findAllSongsByTitle(const TS_SongMap& safeMap, const Title& songTitle)
  -> Songs;

INLIMBO_API_CPP auto findAllSongsByTitleFuzzy(const TS_SongMap& safeMap, const Title& songTitle,
                                              size_t maxDistance = 3) -> Songs;

// Find file path by inode
INLIMBO_API_CPP auto findSongPathByInode(const TS_SongMap& safeMap, const ino_t givenInode)
  -> PathStr;

// Exact title lookup
INLIMBO_API_CPP auto findSongObjByTitle(const TS_SongMap& safeMap, const Title& songTitle)
  -> std::shared_ptr<Song>;

// Fuzzy title lookup
INLIMBO_API_CPP auto findSongObjByTitleFuzzy(const TS_SongMap& safeMap, const Title& songTitle,
                                             size_t maxDistance = 3) -> std::shared_ptr<Song>;

// Fuzzy artist lookup
INLIMBO_API_CPP auto findArtistFuzzy(const TS_SongMap& safeMap, const Artist& artistName,
                                     size_t maxDistance = 2) -> Artist;

// Fuzzy album lookup
INLIMBO_API_CPP auto findAlbumFuzzy(const TS_SongMap& safeMap, const Album& albumName,
                                    size_t maxDistance = 2) -> Album;

// Fuzzy genre lookup
INLIMBO_API_CPP auto findGenreFuzzy(const TS_SongMap& safeMap, const Genre& genreName,
                                    size_t maxDistance = 2) -> Genre;

// Exact artist + title lookup
INLIMBO_API_CPP auto findSongByTitleAndArtist(const TS_SongMap& safeMap, const Artist& artistName,
                                              const Title& songTitle) -> std::shared_ptr<Song>;

// Fuzzy artist + title lookup
INLIMBO_API_CPP auto findSongByTitleAndArtistFuzzy(const TS_SongMap& safeMap,
                                                   const Title& songTitle, const Artist& songArtist,
                                                   size_t maxDistance) -> std::shared_ptr<Song>;

// ==--------------------==
// Aggregates queries
// ==--------------------==

// Get songs belonging to artist
INLIMBO_API_CPP auto getSongsByArtist(const TS_SongMap& safeMap, const Artist& artist) -> Songs;

// Get songs belonging to album
INLIMBO_API_CPP auto getSongsByAlbum(const TS_SongMap& safeMap, const Artist& artist,
                                     const Album& album) -> const Songs;

// Get songs belonging to genre
INLIMBO_API_CPP auto getSongsByGenre(const TS_SongMap& safeMap, const Genre& genre) -> Songs;

// Count songs belonging to artist
INLIMBO_API_CPP auto countSongsByArtist(const TS_SongMap& safeMap, const Artist& artist) -> size_t;

// Count songs belonging to album
INLIMBO_API_CPP auto countSongsByAlbum(const TS_SongMap& safeMap, const Artist& artist,
                                       const Album& album) -> size_t;

// Count songs belonging to genre
INLIMBO_API_CPP auto countSongsByGenre(const TS_SongMap& safeMap, const Genre& genre) -> size_t;

// Count total tracks
INLIMBO_API_CPP auto countTracks(const TS_SongMap& safeMap) -> size_t;

// Count total artists (Unique)
INLIMBO_API_CPP auto countArtists(const TS_SongMap& safeMap) -> size_t;

// Count total albums (Unique)
INLIMBO_API_CPP auto countAlbums(const TS_SongMap& safeMap) -> size_t;

// Count total genres (Unique)
INLIMBO_API_CPP auto countGenres(const TS_SongMap& safeMap) -> size_t;

} // namespace read

namespace mut
{

// Sort entire SongMap using runtime plan
INLIMBO_API_CPP void sortSongMap(TS_SongMap&                         safeMap,
                                 const query::sort::RuntimeSortPlan& rtSortPlan);

// Replace Song object and write metadata to file
INLIMBO_API_CPP auto replaceSongObjAndUpdateMetadata(TS_SongMap&                  safeMap,
                                                     const std::shared_ptr<Song>& oldSong,
                                                     const std::shared_ptr<Song>& newSong,
                                                     taglib::Parser&              parser) -> bool;

} // namespace mut

} // namespace query::songmap
