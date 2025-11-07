#pragma once

#include "Config.hpp"
#include "thread/Map.hpp"   // for threads::SafeMap
#include "core/SongTree.hpp" // for Song, Artist, Album, etc.
#include "String.hpp"
#include <vector>
#include <functional>
#include <iostream>

namespace utils::songmap {

// ---------- COUNTING FUNCTIONS ----------

inline INLIMBO_API_CPP auto countArtists(const threads::SafeMap<dirsort::SongMap>& safeMap) -> size_t {
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::countArtists");
    return safeMap.withReadLock([](const auto& map) {
        return map.size();
    });
}

inline INLIMBO_API_CPP auto countAlbums(const threads::SafeMap<dirsort::SongMap>& safeMap) -> size_t {
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::countAlbums");
    return safeMap.withReadLock([](const auto& map) {
        size_t total = 0;
        for (const auto& [artist, albums] : map)
            total += albums.size();
        return total;
    });
}

inline INLIMBO_API_CPP auto countDiscs(const threads::SafeMap<dirsort::SongMap>& safeMap) -> size_t {
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::countDiscs");
    return safeMap.withReadLock([](const auto& map) {
        size_t total = 0;
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                total += discs.size();
        return total;
    });
}

inline INLIMBO_API_CPP auto countTracks(const threads::SafeMap<dirsort::SongMap>& safeMap) -> size_t {
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::countTracks");
    return safeMap.withReadLock([](const auto& map) {
        size_t total = 0;
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    total += tracks.size();
        return total;
    });
}

// ---------- FETCH FUNCTIONS (Selective) ----------

inline INLIMBO_API_CPP auto findSongByName(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::string& songName) -> const dirsort::Song*
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::findSongByName");
    return safeMap.withReadLock([&](const auto& map) -> const dirsort::Song* {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, song] : tracks)
                        if (string::iequals_fast(song.metadata.title, songName))
                            return &song;
        return nullptr;
    });
}

inline INLIMBO_API_CPP auto findSongByNameAndArtist(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::string& artistName,
    const std::string& songName) -> const dirsort::Song*
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::findSongByNameAndArtist");
    return safeMap.withReadLock([&](const auto& map) -> const dirsort::Song* {
        for (const auto& [artist, albums] : map) {
            if (!string::iequals_fast(artist, artistName))
                continue;
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, song] : tracks)
                        if (string::iequals_fast(song.metadata.title, songName))
                            return &song;
        }
        return nullptr;
    });
}

inline INLIMBO_API_CPP auto getAlbumsByArtist(
    const threads::SafeMap<dirsort::SongMap>& safeMap, const Artist& artist) -> Albums
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::getAlbumsByArtist");
    return safeMap.withReadLock([&](const auto& map) {
        std::vector<Album> albums;
        for (const auto& [a, albumMap] : map) {
            if (string::iequals_fast(a, artist)) {
                albums.reserve(albumMap.size());
                for (const auto& [album, _] : albumMap)
                    albums.push_back(album);
                break;
            }
        }
        return albums;
    });
}

inline INLIMBO_API_CPP auto getSongsByAlbum(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const Artist& artist, const Album& album) -> std::vector<dirsort::Song>
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::getSongsByAlbum");
    return safeMap.withReadLock([&](const auto& map) {
        std::vector<dirsort::Song> songs;
        for (const auto& [a, albums] : map) {
            if (!string::iequals_fast(a, artist))
                continue;
            for (const auto& [alb, discs] : albums) {
                if (!string::iequals_fast(alb, album))
                    continue;
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, song] : tracks)
                        songs.push_back(song);
                return songs;
            }
        }
        return songs;
    });
}

// ---------- FETCH FUNCTIONS ----------

inline INLIMBO_API_CPP auto getAllArtists(const threads::SafeMap<dirsort::SongMap>& safeMap) -> Artists {
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::getAllArtists");
    return safeMap.withReadLock([](const auto& map) {
        std::vector<Artist> artists;
        artists.reserve(map.size());
        for (const auto& [artist, _] : map)
            artists.push_back(artist);
        return artists;
    });
}

inline INLIMBO_API_CPP auto getAllAlbums(const threads::SafeMap<dirsort::SongMap>& safeMap) -> Albums {
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::getAllAlbums");
    return safeMap.withReadLock([](const auto& map) {
        std::vector<Album> albums;
        for (const auto& [artist, albumMap] : map)
            for (const auto& [album, _] : albumMap)
                albums.push_back(album);
        return albums;
    });
}

inline INLIMBO_API_CPP auto getAllGenres(
    const threads::SafeMap<dirsort::SongMap>& safeMap) -> Genres
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::getALLGenres");
    return safeMap.withReadLock([&](const auto& map) {
        std::set<Genre> genreSet;

        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, song] : tracks)
                        genreSet.insert(song.metadata.genre);

        return Genres(genreSet.begin(), genreSet.end());
    });
}

// ---------- FULL TRAVERSAL ----------

inline INLIMBO_API_CPP void forEachArtist(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const std::map<Album,
                                std::map<Disc, std::map<Track, dirsort::Song>>>&)>& fn)
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::forEachArtist");
    safeMap.withReadLock([&](const auto& map) {
        for (const auto& [artist, albums] : map)
            fn(artist, albums);
    });
}


inline INLIMBO_API_CPP void forEachAlbum(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const Album&,
                             const std::map<Disc, std::map<Track, dirsort::Song>>&)>& fn)
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::forEachAlbum");
    safeMap.withReadLock([&](const auto& map) {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                fn(artist, album, discs);
    });
}


inline INLIMBO_API_CPP void forEachSong(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const Artist&, const Album&, Disc, Track,
                             const dirsort::Song&)>& fn)
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::forEachSong");
    safeMap.withReadLock([&](const auto& map) {
        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, song] : tracks)
                        fn(artist, album, disc, track, song);
    });
}


inline INLIMBO_API_CPP void forEachGenre(
    const threads::SafeMap<dirsort::SongMap>& safeMap,
    const std::function<void(const std::string& genre,
                             const std::vector<const dirsort::Song*>& songs)>& fn)
{
    RECORD_FUNC_TO_BACKTRACE("utils::songmap::forEachGenre");
    safeMap.withReadLock([&](const auto& map) {
        BucketedMap<Genre, const dirsort::Song*> genreBuckets;

        for (const auto& [artist, albums] : map)
            for (const auto& [album, discs] : albums)
                for (const auto& [disc, tracks] : discs)
                    for (const auto& [track, song] : tracks)
                        genreBuckets[song.metadata.genre].push_back(&song);

        for (const auto& [genre, songs] : genreBuckets)
            fn(genre, songs);
    });
}

// ---------- PRINT SUMMARY ----------

inline void printSummary(const threads::SafeMap<dirsort::SongMap>& safeMap) {
    safeMap.withReadLock([](const auto& map) {
        std::cout << "-------------------------------------------\n";
        std::cout << "Library Summary\n";
        std::cout << "-------------------------------------------\n";

        size_t totalAlbums = 0, totalDiscs = 0, totalTracks = 0;

        for (const auto& [artist, albums] : map) {
            std::cout << "🎵 " << artist << " (" << albums.size() << " albums)\n";
            totalAlbums += albums.size();
            for (const auto& [album, discs] : albums) {
                std::cout << "   📀 " << album << " (" << discs.size() << " discs)\n";
                totalDiscs += discs.size();
                for (const auto& [disc, tracks] : discs) {
                    std::cout << "      💿 Disc " << disc << " (" << tracks.size() << " tracks)\n";
                    totalTracks += tracks.size();
                }
            }
        }

        std::cout << "-------------------------------------------\n";
        std::cout << "Artists: " << map.size()
                  << " | Albums: " << totalAlbums
                  << " | Discs: " << totalDiscs
                  << " | Tracks: " << totalTracks
                  << "\n-------------------------------------------\n";
    });
}

} // namespace utils::songmap
