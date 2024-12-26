#ifndef SONG_MAP_HPP
#define SONG_MAP_HPP

#include "taglib_parser.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Song
{
  unsigned int inode;
  Metadata     metadata;

  Song(unsigned int inode, const Metadata& metadata) : inode(inode), metadata(metadata) {}

  Song() : inode(0), metadata() {}
};

class SongTree
{
private:
  // Nested map structure: Artist -> Album -> Disc -> Track -> Song
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    tree;

public:
  // Add a song to the tree
  void addSong(const Song& song)
  {
    tree[song.metadata.artist][song.metadata.album][song.metadata.discNumber][song.metadata.track] =
      song;
  }

  // Display all songs hierarchically
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

  // Retrieve songs for a specific artist
  std::vector<Song> getSongsByArtist(const std::string& artist) const
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

  // Retrieve songs for a specific album of an artist
  std::vector<Song> getSongsByAlbum(const std::string& artist, const std::string& album) const
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
};

#endif
