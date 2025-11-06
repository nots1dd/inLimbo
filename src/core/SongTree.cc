#include "core/SongTree.hpp"

namespace dirsort
{

// ============================================================
// Song Implementations
// ============================================================

Song::Song(unsigned int inode, Metadata metadata)
    : inode(inode), metadata(std::move(metadata))
{
}

Song::Song() : inode(0), metadata() {}

// ============================================================
// SongTree Implementations
// ============================================================

void SongTree::addSong(const Song& song)
{
    auto& artistMap = tree[song.metadata.artist];
    auto& albumMap  = artistMap[song.metadata.album];
    auto& discMap   = albumMap[song.metadata.discNumber];
    discMap[song.metadata.track] = song;
}

// ------------------------------------------------------------
void SongTree::display(DisplayMode mode) const
{
    RECORD_FUNC_TO_BACKTRACE("Dirsort");

    const bool print_tree = (mode == DisplayMode::FullTree);
    int totalArtists = 0, totalAlbums = 0, totalDiscs = 0, totalSongs = 0;
    std::set<Genre> uniqueGenres;

    if (print_tree)
        std::cout << "─────────────────────────────────────────────────────────────────────────────\n";

    for (const auto& artistPair : tree)
    {
        totalArtists++;
        if (print_tree)
            std::cout << "\nArtist: " << artistPair.first << "\n";

        for (const auto& albumPair : artistPair.second)
        {
            totalAlbums++;
            if (print_tree)
                std::cout << "  ├─── Album: " << albumPair.first << "\n";

            for (const auto& discPair : albumPair.second)
            {
                totalDiscs++;
                if (print_tree)
                    std::cout << "  │    Disc " << discPair.first << "\n";

                for (const auto& trackPair : discPair.second)
                {
                    totalSongs++;
                    const auto& song = trackPair.second;
                    uniqueGenres.insert(song.metadata.genre);

                    if (print_tree)
                        std::cout << "  │    │    Track " << std::setw(2)
                                  << trackPair.first << ": " << song.metadata.title << "\n";
                }
            }
        }
    }

    std::cout << "──────────────────────────────── Library Summary ────────────────────────────\n";
    std::cout << "Total Artists: " << totalArtists << "\n";
    std::cout << "Total Albums : " << totalAlbums << "\n";
    std::cout << "Total Discs  : " << totalDiscs << "\n";
    std::cout << "Total Songs  : " << totalSongs << "\n";
    std::cout << "Unique Genres: " << uniqueGenres.size() << "\n";
    std::cout << "─────────────────────────────────────────────────────────────────────────────\n";
}

// ------------------------------------------------------------
void SongTree::printAllArtists() const
{
    std::cout << "\n──────────────────────────────── All Artists ────────────────────────────\n";
    for (const auto& artistPair : tree)
        std::cout << "- " << artistPair.first << "\n";
    std::cout << "────────────────────────────────────────────────────────────────────────────\n";
}

// ------------------------------------------------------------
void SongTree::printSongs(const std::vector<Song>& songs)
{
    if (songs.empty())
    {
        std::cout << "No songs found.\n";
        return;
    }

    std::map<std::string, std::vector<Song>> albums;
    for (const auto& song : songs)
        albums[song.metadata.album].push_back(song);

    std::cout << "\nSongs List:\n";
    std::cout << "─────────────────────────────────────\n";

    for (const auto& albumPair : albums)
    {
        const auto& album      = albumPair.first;
        const auto& albumSongs = albumPair.second;

        std::cout << "├─── Album: " << album << "\n";
        std::cout << "└─ Total Songs: " << albumSongs.size() << "\n";

        for (const auto& song : albumSongs)
            std::cout << "    │  Track: " << song.metadata.title << "\n";

        std::cout << "─────────────────────────────────────\n";
    }
}

// ------------------------------------------------------------
auto SongTree::getSongsByArtist(const std::string& artist)
{
    std::vector<Song> result;
    auto artistIt = tree.find(artist);
    if (artistIt != tree.end())
    {
        for (const auto& albumPair : artistIt->second)
            for (const auto& discPair : albumPair.second)
                for (const auto& trackPair : discPair.second)
                    result.push_back(trackPair.second);
    }
    printSongs(result);
    return result;
}

// ------------------------------------------------------------
auto SongTree::getSongsByAlbum(const std::string& artist, const std::string& album) const
{
    std::vector<Song> result;
    auto artistIt = tree.find(artist);
    if (artistIt != tree.end())
    {
        auto albumIt = artistIt->second.find(album);
        if (albumIt != artistIt->second.end())
        {
            for (const auto& discPair : albumIt->second)
                for (const auto& trackPair : discPair.second)
                    result.push_back(trackPair.second);
        }
    }
    return result;
}

// ------------------------------------------------------------
void SongTree::getSongsByGenreAndPrint() const
{
    std::map<std::string, std::vector<Song>> genreMap;

    for (const auto& artistPair : tree)
        for (const auto& albumPair : artistPair.second)
            for (const auto& discPair : albumPair.second)
                for (const auto& trackPair : discPair.second)
                    genreMap[trackPair.second.metadata.genre].push_back(trackPair.second);

    std::cout << "\n=== Songs Grouped by Genre ===\n";
    std::cout << "──────────────────────────────────────\n";

    for (const auto& genrePair : genreMap)
    {
        const auto& genre = genrePair.first;
        const auto& songs = genrePair.second;

        std::cout << "\nGenre: " << genre << "\n";
        std::cout << "──────────────────────────────────────\n";

        for (const auto& song : songs)
        {
            std::cout << "├─── " << song.metadata.title
                      << " by " << song.metadata.artist
                      << " (Album: " << song.metadata.album << ")\n";
        }
        std::cout << "──────────────────────────────────────\n";
    }
}

// ------------------------------------------------------------
void SongTree::saveToFile(const std::string& filename) const
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file for saving.");

    cereal::BinaryOutputArchive archive(file);
    archive(*this);
}

// ------------------------------------------------------------
void SongTree::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file for loading.");

    cereal::BinaryInputArchive archive(file);
    archive(*this);
}

// ------------------------------------------------------------
void SongTree::printSongInfo(const std::string& input)
{
    bool isFilePath = input.find('/') != std::string::npos || input.find('\\') != std::string::npos;
    std::optional<Song> foundSong;

    if (isFilePath)
    {
        std::cout << "\n> Taking argument as a possible audio file path...\n";
        struct stat fileStat {};
        if (stat(input.c_str(), &fileStat) == 0)
        {
            unsigned int inode = fileStat.st_ino;
            for (const auto& artistPair : tree)
                for (const auto& albumPair : artistPair.second)
                    for (const auto& discPair : albumPair.second)
                        for (const auto& trackPair : discPair.second)
                            if (trackPair.second.inode == inode)
                                foundSong = trackPair.second;
        }
    }
    else
    {
        for (const auto& artistPair : tree)
            for (const auto& albumPair : artistPair.second)
                for (const auto& discPair : albumPair.second)
                    for (const auto& trackPair : discPair.second)
                        if (levenshteinDistance(trackPair.second.metadata.title, input) < 3)
                            foundSong = trackPair.second;
    }

    if (foundSong)
    {
        const auto& song = *foundSong;
        std::cout << "\nSong Information:\n";
        std::cout << "──────────────────────────────────────\n";
        std::cout << "Title     : " << song.metadata.title << "\n";
        std::cout << "Artist    : " << song.metadata.artist << "\n";
        std::cout << "Album     : " << song.metadata.album << "\n";
        std::cout << "Disc      : " << song.metadata.discNumber << "\n";
        std::cout << "Track     : " << song.metadata.track << "\n";
        std::cout << "Genre     : " << song.metadata.genre << "\n";
        std::cout << "Inode     : " << song.inode << "\n";

        if (!song.metadata.additionalProperties.empty())
        {
            std::cout << "Additional Properties:\n";
            for (const auto& prop : song.metadata.additionalProperties)
            {
                if (prop.first == "LYRICS")
                {
                    std::cout << "\nLyrics:\n";
                    std::cout << "──────────────────────────────────────\n";

                    const std::string& lyrics = prop.second;
                    size_t lineLength = 80;
                    for (size_t start = 0; start < lyrics.size(); start += lineLength)
                        std::cout << lyrics.substr(start, std::min(lineLength, lyrics.size() - start)) << "\n";

                    std::cout << "──────────────────────────────────────\n";
                }
                else
                {
                    std::cout << "  - " << prop.first << " : " << prop.second << "\n";
                }
            }
        }
        else
        {
            std::cout << "No additional properties found!\n";
        }
        std::cout << "──────────────────────────────────────\n";
    }
    else
    {
        std::cout << "⚠️  Song not found: " << input << "\n";
    }
}

} // namespace dirsort
