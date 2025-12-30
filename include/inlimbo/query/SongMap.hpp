#pragma once

#include "Config.hpp"
#include "thread/Map.hpp"    // threads::SafeMap
#include "core/SongTree.hpp" // dirsort::Song, Artist, Album, etc.
#include "utils/String.hpp"
#include <vector>
#include <functional>
#include "core/taglib/Parser.hpp"

namespace query::songmap {

namespace strhelp = utils::string;

namespace read {

inline INLIMBO_API_CPP auto findSongByName(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::string& songName
) -> const dirsort::Song*
{
    RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByName");
    return safeMap.withReadLock([&](const auto& map) -> const dirsort::Song* {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, inodeMap] : tracks)
                        for (const auto& [inode, song] : inodeMap)
                            if (strhelp::iequals_fast(song.metadata.title, songName))
                                return &song;
        return nullptr;
    });
}

inline INLIMBO_API_CPP auto findSongByNameAndArtist(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const Artist& artistName,
    const std::string& songName
) -> const dirsort::Song*
{
    RECORD_FUNC_TO_BACKTRACE("query::songmap::read::findSongByNameAndArtist");
    return safeMap.withReadLock([&](const auto& map) -> const dirsort::Song* {
        for (const auto& [artist, albums] : map) {
            if (!strhelp::iequals_fast(artist, artistName))
                continue;
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, inodeMap] : tracks)
                        for (const auto& [inode, song] : inodeMap)
                            if (strhelp::iequals_fast(song.metadata.title, songName))
                                return &song;
        }
        return nullptr;
    });
}

inline INLIMBO_API_CPP auto getSongsByAlbum(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const Artist& artist,
    const Album& album
) -> const dirsort::Songs
{
    RECORD_FUNC_TO_BACKTRACE("query::songmap::read::getSongsByAlbum");
    return safeMap.withReadLock([&](const auto& map) -> const dirsort::Songs {
        dirsort::Songs songs;
        for (const auto& [a, albums] : map) {
            if (!strhelp::iequals_fast(a, artist))
                continue;
            for (const auto& [alb, discs] : albums) {
                if (!strhelp::iequals_fast(alb, album))
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

inline INLIMBO_API_CPP auto countTracks(
    const threads::SafeMap<dirsort::SongMap>& safeMap
) -> size_t
{
    RECORD_FUNC_TO_BACKTRACE("query::songmap::read::countTracks");
    return safeMap.withReadLock([](const auto& map) -> size_t {
        size_t total = 0;
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, inodeMap] : tracks)
                        total += inodeMap.size();
        return total;
    });
}

// ------------------------------------------------------------
// forEach helpers
// ------------------------------------------------------------

inline INLIMBO_API_CPP void forEachArtist(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const dirsort::AlbumMap&)>& fn
)
{
    safeMap.withReadLock([&](const auto& map) -> void {
        for (const auto& [artist, albums] : map)
            fn(artist, albums);  // AlbumMap is still Album -> Disc -> Track -> inode -> Song
    });
}

inline INLIMBO_API_CPP void forEachAlbum(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const Album&, const dirsort::DiscMap&)>& fn
)
{
    safeMap.withReadLock([&](const auto& map) -> void {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                fn(artist, album, discs); // DiscMap is still Disc -> Track -> inode -> Song
    });
}

inline INLIMBO_API_CPP void forEachDisc(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const Album&, const Disc disc, const dirsort::TrackMap&)>& fn
)
{
    safeMap.withReadLock([&](const auto& map) -> void {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    fn(artist, album, disc, tracks); 
                    // tracks is now Track -> inode -> Song
    });
}

inline INLIMBO_API_CPP void forEachSong(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const Album&, const Disc disc, const Track track, const ino_t inode, const dirsort::Song&)>& fn
)
{
    safeMap.withReadLock([&](const auto& map) -> void {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, inodeMap] : tracks)
                        for (const auto& [inode, song] : inodeMap)
                            fn(artist, album, disc, track, inode, song);
    });
}

} // namespace read

namespace mut {

inline INLIMBO_API_CPP auto replaceSongObjAndUpdateMetadata(
    threads::SafeMap<dirsort::SongMap>& safeMap,
    const dirsort::Song& oldSong,
    const dirsort::Song& newSong,
    TagLibParser& parser
) -> bool
{
    RECORD_FUNC_TO_BACKTRACE("query::songmap::mut::replaceSongObjByInode");

    ASSERT_MSG(oldSong.inode == newSong.inode, "Inode details changed! Will not be writing...");
    LOG_INFO("Song map size: {}", safeMap.snapshot().size());

    return safeMap.withWriteLock([&](auto& map) -> bool {
        for (auto& [artist, albums] : map) {
            for (auto& [album, discs] : albums) {
                for (auto& [disc, tracks] : discs) {
                    for (auto& [track, inodeMap] : tracks) {
                        for (auto& [inodeKey, song] : inodeMap) {
                            if (inodeKey == oldSong.inode) {
                                LOG_INFO("Match found → Artist='{}', Album='{}', Disc={}, Track={}, Inode={}",
                                         artist, album, disc, track, inodeKey);

                                song = newSong;
                                LOG_DEBUG("Replaced song object for inode={} successfully.", inodeKey);

                                // Update metadata on disk
                                if (parser.modifyMetadata(newSong.metadata.filePath, newSong.metadata)) {
                                    LOG_INFO("Successfully updated metadata on disk for file: {}",
                                             newSong.metadata.filePath);
                                    return true;
                                } else {
                                    LOG_ERROR("Failed to update metadata on disk for file: {}",
                                              newSong.metadata.filePath);
                                    return false;
                                }
                            }
                        }
                    }
                }
            }
        }

        LOG_WARN("Inode '{}' not found in SongMap.", oldSong.inode);
        return false;
    });
}

} // namespace mut

} // namespace query::songmap
