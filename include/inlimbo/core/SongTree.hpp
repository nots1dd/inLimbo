#pragma once

#include "taglib/Parser.hpp"
#include "StackTrace.hpp"
#include "helpers/levenshtein.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
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
    unsigned int inode;    /**< The inode of the file representing the song */
    Metadata     metadata; /**< Metadata information for the song */

    Song(unsigned int inode, Metadata metadata);
    Song();

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(inode, metadata);
    }
};

// ============================================================
// SongTree Declaration
// ============================================================
using SongMap = std::map<Artist, std::map<Album, std::map<Disc, std::map<Track, Song>>>>;

class SongTree
{
private:
    SongMap tree;
    std::string musicPath;

public:
    // Core methods
    void addSong(const Song& song);
    void display(DisplayMode mode = DisplayMode::Summary) const;
    void setMusicPath(const std::string& path) { musicPath = path; }
    
    void clear()
    {
        RECORD_FUNC_TO_BACKTRACE("SongTree::clear");
        tree.clear();
        musicPath.clear();
    }

    // Query methods
    void printAllArtists() const;
    void printSongs(const std::vector<Song>& songs);
    auto getSongsByArtist(const std::string& artist);
    [[nodiscard]] auto getSongsByAlbum(const std::string& artist, const std::string& album) const;
    void getSongsByGenreAndPrint() const;
    [[nodiscard]] auto returnSongMap() const { return tree; }
    [[nodiscard]] auto returnMusicPath() const { return musicPath; }

    // Persistence
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(tree, musicPath);
    }

    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);

    // Info
    void printSongInfo(const std::string& input);
};

} // namespace dirsort
