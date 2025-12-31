#pragma once

#include "taglib/Parser.hpp"
#include "StackTrace.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <map>
#include <string>
#include <vector>
#include <sys/stat.h> // for inode/stat lookup

namespace core
{

// ============================================================
// Display Mode
// ============================================================
enum class DisplayMode {
    Summary,
    FullTree
};

// ============================================================
// Song Structure
// ============================================================
struct Song
{
    uint inode;    /**< The inode of the file representing the song */
    Metadata     metadata; /**< Metadata information for the song */

    Song(uint inode, Metadata metadata);
    Song();

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(inode, metadata);
    }
};

// 
// NOTE:
//
// If you have the entire song map, use the inode map (that is NOT in the Song structure)
//
// If you only have the Song struct at your disposal (say for printing, etc.) just Song.inode instead.
//
// Theoretically there should be no difference between the two but this can be a bit confusing.
//
// This was initially done to account for duplicated metadata files.
//
using SongMap = std::map<Artist, std::map<Album, std::map<Disc, std::map<Track ,std::map<ino_t, Song>>>>>;
using Songs = std::vector<Song>;

using InodeMap = std::map<ino_t, Song>;
using TrackMap = std::map<Track, InodeMap>;
using DiscMap  = std::map<Disc, TrackMap>;
using AlbumMap = std::map<Album, DiscMap>;

enum class VisitResult {
    Continue,
    Stop
};

using SongTreeVisitor =
    std::function<VisitResult(
        const Artist&,
        const Album&,
        Disc,
        Track,
        const Song&
    )>;

using ArtistVisitor = std::function<VisitResult(const Artist&)>;
using AlbumVisitor  = std::function<VisitResult(const Artist&, const Album&)>;
using DiscVisitor   = std::function<VisitResult(const Artist&, const Album&, Disc)>;

struct SongTreeCallbacks {
    std::function<void()> onBegin = {};
    ArtistVisitor         onArtist = {};
    AlbumVisitor          onAlbum = {};
    DiscVisitor           onDisc = {};
    SongTreeVisitor       onSong = {};
    std::function<void(
        int artists,
        int albums,
        int discs,
        int songs,
        int uniqueGenres
    )> onSummary = {};
    std::function<void()> onEnd = {};
};

using SongPredicate =
    std::function<bool(
        const Artist&,
        const Album&,
        Disc,
        Track,
        const Song&
    )>;

namespace song::sort 
{

inline auto byArtist(const Artist& a) -> SongPredicate
{
    return [a](const Artist& artist, auto&, auto, auto, auto&) -> bool {
        return artist == a;
    };
}

inline auto byAlbum(const Artist& a, const Album& al) -> SongPredicate
{
    return [a, al](const Artist& artist, const Album& album, auto, auto, auto&) -> bool {
        return artist == a && album == al;
    };
}

inline auto byGenre(const Genre& g) -> SongPredicate
{
    return [g](auto&, auto&, auto, auto, const Song& s) -> bool {
        return s.metadata.genre == g;
    };
}

inline auto allOf(SongPredicate a, SongPredicate b) -> SongPredicate
{
    return [=](auto&&... args) -> auto {
        return a(args...) && b(args...);
    };
}

inline auto anyOf(SongPredicate a, SongPredicate b) -> SongPredicate
{
    return [=](auto&&... args) -> auto {
        return a(args...) || b(args...);
    };
}

inline auto notOf(SongPredicate p) -> SongPredicate
{
    return [=](auto&&... args) -> auto {
        return !p(args...);
    };
}

} // namespace song::sort

// ============================================================
// SongTree Declaration (NOT THREAD SAFE)
// ============================================================
//
// Note that this structure is used ONLY for serialization and deserialization.
// The in-memory representation used during runtime is a threads::SafeMap<SongMap>.
//
// This is majorly also used for cmdline querying and printing the song library.

class SongTreeRange;

class SongTree
{
private:
    SongMap m_songMap;
    std::string m_musicPath;    
    
public:
    // Core methods
    void addSong(const Song& song);
    void traverse(const SongTreeCallbacks& cb) const;
    auto findSong(
    const std::function<bool(const Song&)>& predicate,
    const std::function<void(const Song&)>& onFound
    ) const -> bool;
    void forEach(
    const SongPredicate& pred,
    const SongTreeVisitor& visitor
    ) const;
    [[nodiscard]] auto range(SongPredicate pred) const -> SongTreeRange;
    void setMusicPath(const std::string& path) { m_musicPath = path; }
    
    void clear()
    {
        RECORD_FUNC_TO_BACKTRACE("SongTree::clear");
        m_songMap.clear();
        m_musicPath.clear();
    }

    // Query methods
    [[nodiscard]] auto returnSongMap() const { return m_songMap; }
    // note that this replaces the entire song map and newMap is no longer valid after this call
    [[nodiscard]] auto newSongMap(const SongMap& newMap)
    {
        RECORD_FUNC_TO_BACKTRACE("SongTree::replaceSongMap");
        m_songMap = std::move(newMap);
    }
    [[nodiscard]] auto returnMusicPath() const { return m_musicPath; }

    // Persistence
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(m_songMap, m_musicPath);
    }

    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
};

} // namespace core
