/**
 * @file SongMap.hpp
 * @brief Contains the definitions for the Song and SongTree classes used to manage and store song
 * metadata.
 *
 * This file defines the structure for storing and retrieving song data, including metadata such as
 * artist, album, track, and disc number. The songs are organized within a hierarchical tree
 * structure and can be serialized and deserialized using the Cereal library for storage or
 * transmission.
 *
 * The Song class represents a single song, with its metadata and inode. The SongTree class
 * organizes multiple songs in a nested map structure for efficient retrieval by artist, album,
 * track, and disc number.
 *
 * @note This file depends on the `TagLibParser.h` for the `Metadata` struct, which is used within
 * the `Song` class.
 *
 * @see TagLibParser.h
 */

#pragma once

#include "taglib_parser.h"
#include "../helpers/levenshtein.hpp"
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

/**
 * @struct Song
 * @brief Represents a song with associated metadata and inode.
 *
 * The `Song` struct encapsulates the metadata and inode of a song, including details such as title,
 * artist, album, genre, and track number. It provides methods to initialize, serialize, and
 * deserialize song data.
 */
struct Song
{
  unsigned int inode;    /**< The inode of the file representing the song */
  Metadata     metadata; /**< Metadata information for the song */

  /**
   * @brief Constructs a Song with the given inode and metadata.
   *
   * This constructor initializes a song with a unique inode and metadata.
   *
   * @param inode The inode of the song file.
   * @param metadata The metadata associated with the song.
   */
  Song(unsigned int inode, const Metadata& metadata) : inode(inode), metadata(metadata) {}

  /**
   * @brief Default constructor for a Song, initializing with default values.
   */
  Song() : inode(0), metadata() {}

  /**
   * @brief Serializes the Song object.
   *
   * This function is used by the Cereal library to serialize the Song object for storage or
   * transmission.
   *
   * @param ar The archive object used for serialization.
   */
  template <class Archive> void serialize(Archive& ar) { ar(inode, metadata); }
};

/**
 * @class SongTree
 * @brief Represents a hierarchical tree structure to store songs by artist, album, disc number, and
 * track number.
 *
 * The `SongTree` class organizes songs in a nested map structure, allowing for efficient storage
 * and retrieval of songs based on artist, album, disc, and track numbers. It supports the addition,
 * display, and serialization of songs, and provides methods for querying songs by specific criteria
 * such as artist, album, and genre.
 *
 * Songs are added to the tree via the `addSong` method, and the tree can be serialized and
 * deserialized to a file using the Cereal library.
 */
class SongTree
{
private:
  /**
   * @brief The nested map structure to store songs.
   *
   * The map is organized as follows:
   * Artist -> Album -> Disc -> Track -> Song
   *
   * The `Song` objects are stored in the innermost map, organized by their respective track number.
   */
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    tree;

public:
  /**
   * @brief Adds a song to the tree.
   *
   * This method inserts a song into the appropriate location in the tree based on its artist,
   * album, disc number, and track number.
   *
   * @param song The song to be added to the tree.
   */
  void addSong(const Song& song)
  {
    tree[song.metadata.artist][song.metadata.album][song.metadata.discNumber][song.metadata.track] =
      song;
  }

  /**
   * @brief Displays all songs in the tree hierarchically.
   *
   * This method prints the entire song tree to the console, organized by artist, album, disc
   * number, and track number. It also provides a summary of the library, including the total number
   * of artists, albums, discs, songs, and unique genres.
   */
  void display() const
  {
    int                   totalArtists = 0, totalAlbums = 0, totalDiscs = 0, totalSongs = 0;
    std::set<std::string> uniqueGenres; // To count unique genres

    std::cout << "\n=== Music Library Overview ===\n";
    std::cout << "─────────────────────────────────────────────────────────────────────────────\n";

    for (const auto& artistPair : tree)
    {
      totalArtists++;
      std::cout << "\n🎤 Artist: " << artistPair.first << "\n";
      for (const auto& albumPair : artistPair.second)
      {
        totalAlbums++;
        std::cout << "  ├─── 📀 Album: " << albumPair.first << "\n";
        for (const auto& discPair : albumPair.second)
        {
          totalDiscs++;
          std::cout << "  │    💿 Disc " << discPair.first << "\n";
          for (const auto& trackPair : discPair.second)
          {
            totalSongs++;
            const auto& song = trackPair.second;
            uniqueGenres.insert(song.metadata.genre);
            std::cout << "  │    │    Track " << std::setw(2) << trackPair.first << ": "
                      << song.metadata.title << "\n";
          }
        }
      }
    }

    std::cout << "\n=== Library Summary ===\n";
    std::cout << "─────────────────────────────────────────────────────────────────────────────\n";
    std::cout << "🎤 Total Artists: " << totalArtists << "\n";
    std::cout << "📀 Total Albums: " << totalAlbums << "\n";
    std::cout << "💿 Total Discs: " << totalDiscs << "\n";
    std::cout << "🎵 Total Songs: " << totalSongs << "\n";
    std::cout << "🎼 Unique Genres: " << uniqueGenres.size() << "\n";
    std::cout << "=========================\n";
  }

  /**
   * @brief Prints all the artists in the song tree.
   *
   * This method prints a list of all artists present in the song tree.
   */
  void printAllArtists() const
  {
    std::cout << "\n=== Artists List ===\n";
    for (const auto& artistPair : tree)
    {
      std::cout << "- " << artistPair.first << "\n";
    }
    std::cout << "=====================\n";
  }

  /**
   * @brief Prints the songs by album.
   *
   * This method groups the songs by album and prints them in a readable format.
   *
   * @param songs A vector of Song objects to be displayed.
   */
  void printSongs(const std::vector<Song>& songs)
  {
    if (songs.empty())
    {
      std::cout << "No songs found.\n";
      return;
    }

    // Group songs by album
    std::map<std::string, std::vector<Song>> albums;
    for (const auto& song : songs)
    {
      albums[song.metadata.album].push_back(song);
    }

    std::cout << "\n" << "\033[1mSongs List\033[0m:\n";
    std::cout << "─────────────────────────────────────\n";

    for (const auto& albumPair : albums)
    {
      const auto& album      = albumPair.first;
      const auto& albumSongs = albumPair.second;

      std::cout << "├─── 📀 Album: " << album << "\n";
      std::cout << "└─ Total Songs: " << albumSongs.size() << "\n";

      for (const auto& song : albumSongs)
      {
        std::cout << "    │  Track: " << song.metadata.title << "\n";
      }

      std::cout << "─────────────────────────────────────\n";
    }
  }

  /**
   * @brief Retrieves all songs by a specific artist.
   *
   * This method returns a vector of all songs by a given artist, organized by album, disc number,
   * and track number.
   *
   * @param artist The name of the artist whose songs are to be retrieved.
   * @return A vector of `Song` objects by the specified artist.
   */
  auto getSongsByArtist(const std::string& artist)
  {
    std::vector<Song> result;
    auto              artistIt = tree.find(artist);
    if (artistIt != tree.end())
    {
      for (const auto& albumPair : artistIt->second)
      {
        for (const auto& discPair : albumPair.second)
        {
          for (const auto& trackPair : discPair.second)
          {
            result.push_back(trackPair.second);
          }
        }
      }
    }
    printSongs(result);
    return result;
  }

  /**
   * @brief Retrieves all songs from a specific album by a given artist.
   *
   * This method returns a vector of all songs from a particular album by an artist.
   *
   * @param artist The name of the artist whose album's songs are to be retrieved.
   * @param album The title of the album whose songs are to be retrieved.
   * @return A vector of `Song` objects from the specified album by the specified artist.
   */
  [[nodiscard]] auto getSongsByAlbum(const std::string& artist, const std::string& album) const
  {
    std::vector<Song> result;
    auto              artistIt = tree.find(artist);
    if (artistIt != tree.end())
    {
      auto albumIt = artistIt->second.find(album);
      if (albumIt != artistIt->second.end())
      {
        for (const auto& discPair : albumIt->second)
        {
          for (const auto& trackPair : discPair.second)
          {
            result.push_back(trackPair.second);
          }
        }
      }
    }
    return result;
  }

  /**
   * @brief Groups and prints songs by genre.
   *
   * This method groups the songs by genre and prints them in a readable format.
   */
  void getSongsByGenreAndPrint() const
  {
    std::map<std::string, std::vector<Song>> genreMap;

    // Populate genreMap with songs grouped by genre
    for (const auto& artistPair : tree)
    {
      for (const auto& albumPair : artistPair.second)
      {
        for (const auto& discPair : albumPair.second)
        {
          for (const auto& trackPair : discPair.second)
          {
            genreMap[trackPair.second.metadata.genre].push_back(trackPair.second);
          }
        }
      }
    }

    std::cout << "\n=== Songs Grouped by Genre ===\n";
    std::cout << "──────────────────────────────────────\n";

    // Print each genre and its corresponding songs
    for (const auto& genrePair : genreMap)
    {
      const auto& genre = genrePair.first;
      const auto& songs = genrePair.second;

      std::cout << "\n🎶 Genre: " << genre << "\n";
      std::cout << "──────────────────────────────────────\n";

      for (const auto& song : songs)
      {
        std::cout << "├─── " << song.metadata.title << " by " << song.metadata.artist
                  << " (Album: " << song.metadata.album << ")\n";
      }

      std::cout << "──────────────────────────────────────\n";
    }
  }

  /**
   * @brief Returns the internal song map.
   *
   * This method returns the entire nested map structure containing all the songs.
   *
   * @return The nested map structure of songs.
   */
  [[nodiscard]] auto returnSongMap() const { return tree; }

  /**
   * @brief Serializes the SongTree object.
   *
   * This function serializes the `SongTree` object into a binary format for storage or
   * transmission.
   *
   * @param ar The archive object used for serialization.
   */
  template <class Archive> void serialize(Archive& ar) { ar(tree); }

  /**
   * @brief Saves the SongTree to a file.
   *
   * This method saves the serialized `SongTree` object to a binary file.
   *
   * @param filename The name of the file to save the `SongTree` to.
   * @throws std::runtime_error If the file cannot be opened for saving.
   */
  void saveToFile(const std::string& filename) const
  {
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
      throw std::runtime_error("Failed to open file for saving.");
    }
    cereal::BinaryOutputArchive archive(file);
    archive(*this); // Serialize the SongTree
  }

  /**
   * @brief Loads a SongTree from a file.
   *
   * This method loads the serialized `SongTree` object from a binary file.
   *
   * @param filename The name of the file to load the `SongTree` from.
   * @throws std::runtime_error If the file cannot be opened for loading.
   */
  void loadFromFile(const std::string& filename)
  {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
      throw std::runtime_error("Failed to open file for loading.");
    }
    cereal::BinaryInputArchive archive(file);
    archive(*this); // Deserialize the SongTree
  }

  /**
   * @brief Prints metadata and additional properties of a song.
   *
   * This function retrieves a song based on either a file path or a song name
   * and prints all relevant metadata, including title, artist, album, genre,
   * track number, disc number, and inode.
   *
   * @param input The file path or song name to search for.
   */
  void printSongInfo(const std::string& input)
  {
    bool isFilePath = input.find('/') != std::string::npos || input.find('\\') != std::string::npos;

    std::optional<Song> foundSong;

    if (isFilePath)
    {
      // If input is a file path, retrieve its metadata using inode
      std::cout << std::endl << "> Taking argument as a possible audio file path..." << std::endl;
      struct stat fileStat;
      if (stat(input.c_str(), &fileStat) == 0)
      {
        unsigned int inode = fileStat.st_ino;
        for (const auto& artistPair : tree)
        {
          for (const auto& albumPair : artistPair.second)
          {
            for (const auto& discPair : albumPair.second)
            {
              for (const auto& trackPair : discPair.second)
              {
                if (trackPair.second.inode == inode)
                {
                  foundSong = trackPair.second;
                  break;
                }
              }
            }
          }
        }
      }
    }
    else
    {
      // If input is a song name, search by title
      for (const auto& artistPair : tree)
      {
        for (const auto& albumPair : artistPair.second)
        {
          for (const auto& discPair : albumPair.second)
          {
            for (const auto& trackPair : discPair.second)
            {
              if (levenshteinDistance(trackPair.second.metadata.title, input) < 3) // Recommended limit of correction (else it will go bonkers)
              {
                foundSong = trackPair.second;
                break;
              }
            }
          }
        }
      }
    }

    if (foundSong)
    {
      const auto& song = *foundSong;
      std::cout << "\n🎵 Song Information:\n";
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
            std::cout << "\n📜 Lyrics:\n";
            std::cout << "──────────────────────────────────────\n";

            // Wrap and format the lyrics
            const std::string& lyrics = prop.second;
            size_t lineLength = 80;
            size_t start = 0;
            while (start < lyrics.size())
            {
              size_t end = start + lineLength;
              if (end > lyrics.size()) end = lyrics.size();
              std::cout << lyrics.substr(start, end - start) << "\n";
              start = end;
            }
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
        std::cout << "No additional properties found!" << std::endl;
      }
      std::cout << "──────────────────────────────────────\n";
    }
    else
    {
      std::cout << "⚠️  Song not found: " << input << "\n";
    }
  }
};
