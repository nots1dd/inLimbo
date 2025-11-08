#pragma once

#include "Config.hpp"
#include "taglib/Parser.hpp"
#include "StackTrace.hpp"
#include "helpers/levenshtein.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <sys/stat.h> // for inode/stat lookup

namespace dirsort
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

using InodeMap = std::map<ino_t, Song>;
using TrackMap = std::map<Track, InodeMap>;
using DiscMap  = std::map<Disc, TrackMap>;
using AlbumMap = std::map<Album, DiscMap>;

// ============================================================
// SongTree Declaration (NOT THREAD SAFE)
// ============================================================
//
// Note that this structure is used ONLY for serialization and deserialization.
// The in-memory representation used during runtime is a threads::SafeMap<SongMap>.
//
// This is majorly also used for cmdline querying and printing the song library.

class SongTree
{
private:
    SongMap map;
    std::string musicPath;    
    
public:
    // Core methods
    void addSong(const Song& song);
    void display(DisplayMode mode = DisplayMode::Summary) const;
    void setMusicPath(const std::string& path) { musicPath = path; }
    
    void clear()
    {
        RECORD_FUNC_TO_BACKTRACE("SongTree::clear");
        map.clear();
        musicPath.clear();
    }

    // Query methods
    void printAllArtists() const;
    void printSongs(const std::vector<Song>& songs);
    auto getSongsByArtist(const std::string& artist);
    [[nodiscard]] auto getSongsByAlbum(const std::string& artist, const std::string& album) const;
    void getSongsByGenreAndPrint() const;
    [[nodiscard]] auto returnSongMap() const { return map; }
    // note that this replaces the entire song map and newMap is no longer valid after this call
    [[nodiscard]] auto newSongMap(const SongMap& newMap)
    {
        RECORD_FUNC_TO_BACKTRACE("SongTree::replaceSongMap");
        map = std::move(newMap);
    }
    [[nodiscard]] auto returnMusicPath() const { return musicPath; }

    // Persistence
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(map, musicPath);
    }

    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);

    // Info
    void printSongInfo(const std::string& input);
};

} // namespace dirsort
