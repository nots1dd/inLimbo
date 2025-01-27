/**
 * @file SongMap.hpp
 * @brief Contains the definitions for the Song and SongTree classes used to manage and store song
 * metadata.
 *
 * This file provides a structure for storing and retrieving song data, including metadata such as
 * artist, album, track, and disc number. The songs are stored in a hierarchical tree structure and
 * can be serialized and deserialized using the Cereal library.
 *
 * The Song class represents a single song with metadata, while the SongTree class organizes songs
 * in a nested map structure.
 *
 * @note This file depends on the taglib_parser.h for the Metadata struct, which is used in the Song
 * class.
 *
 * @see TagLibParser.h
 */

#ifndef SONG_MAP_HPP
#define SONG_MAP_HPP

#include "taglib_parser.h"
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/**
 * @struct Song
 * @brief Represents a song with associated metadata and inode.
 *
 * The Song struct contains an inode identifier and a Metadata object that holds the metadata for a
 * song. The metadata includes details such as title, artist, album, genre, and track number.
 */
struct Song
{
  unsigned int inode;    /**< The inode of the file representing the song */
  Metadata     metadata; /**< Metadata information for the song */

  /**
   * @brief Constructs a Song with the given inode and metadata.
   *
   * @param inode The inode of the song.
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
   * This function is used by the Cereal library to serialize the Song object for saving to a file
   * or transmission.
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
 * The SongTree class organizes songs in a map structure where the keys are artist names, album
 * titles, disc numbers, and track numbers. The tree structure allows for easy retrieval of songs by
 * artist, album, and track number. Songs can be added to the tree, displayed, and serialized or
 * deserialized from a file.
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
   * The Song objects are stored in the innermost map, organized by their respective track number.
   */
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    tree;

public:
  /**
   * @brief Adds a song to the tree.
   *
   * This function inserts a Song into the appropriate location in the tree based on its artist,
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
   * This function prints the entire song tree to the console in a human-readable format, displaying
   * the artist, album, disc number, track number, title, and other metadata for each song.
   */
  void display() const
  {
    for (const auto& artistPair : tree)
    {
      std::cout << "Artist: " << artistPair.first << "\n";
      for (const auto& albumPair : artistPair.second)
      {
        std::cout << "  Album: " << albumPair.first << "\n";
        for (const auto& discPair : albumPair.second)
        {
          std::cout << "    Disc: " << discPair.first << "\n";
          for (const auto& trackPair : discPair.second)
          {
            const auto& song = trackPair.second;
            std::cout << "      Track " << trackPair.first << ": " << song.metadata.title
                      << " (Inode: " << song.inode << ", Artist: " << song.metadata.artist
                      << ", Album: " << song.metadata.album << ", Genre: " << song.metadata.genre
                      << ")\n";
          }
        }
      }
    }
  }

  /**
   * @brief Retrieves all songs by a specific artist.
   *
   * This function returns a vector of all songs by a given artist, organized by album, disc number,
   * and track number.
   *
   * @param artist The name of the artist whose songs are to be retrieved.
   * @return A vector of Song objects by the specified artist.
   */
  auto getSongsByArtist(const std::string& artist) const
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
    return result;
  }

  /**
   * @brief Retrieves all songs from a specific album by a given artist.
   *
   * This function returns a vector of all songs from a particular album by an artist.
   *
   * @param artist The name of the artist whose album's songs are to be retrieved.
   * @param album The title of the album whose songs are to be retrieved.
   * @return A vector of Song objects from the specified album by the specified artist.
   */
  auto getSongsByAlbum(const std::string& artist, const std::string& album) const
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
   * @brief Returns the internal song map.
   *
   * This function returns the entire nested map structure containing all the songs.
   *
   * @return The nested map structure of songs.
   */
  auto returnSongMap() { return tree; }

  /**
   * @brief Serializes the SongTree object.
   *
   * This function serializes the SongTree object into a binary format for storage or transmission.
   *
   * @param ar The archive object used for serialization.
   */
  template <class Archive> void serialize(Archive& ar) { ar(tree); }

  /**
   * @brief Saves the SongTree to a file.
   *
   * This function saves the serialized SongTree object to a binary file.
   *
   * @param filename The name of the file to save the SongTree to.
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
   * This function loads the serialized SongTree object from a binary file.
   *
   * @param filename The name of the file to load the SongTree from.
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
};

#endif
