#pragma once

#include "utils/string/SmallString.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using ui8  = std::uint8_t;
using ui16 = std::uint16_t;
using ui32 = std::uint32_t;
using ui64 = std::uint64_t;

using cstr = const char*;
using vptr = void*;

using strvec = std::vector<std::string>;

// By default, Path and Directory alias to custom SmallString interface.
//
// To use the standard string, use PathStr / DirectoryStr.
using Path      = utils::string::SmallString;
using Directory = utils::string::SmallString;

using Paths = std::vector<Path>;

using PathStr      = std::string;
using DirectoryStr = std::string;

using PathCStr      = const char*;
using DirectoryCStr = const char*;

using Title      = std::string;
using Album      = std::string;
using Artist     = std::string;
using Genre      = std::string;
using Lyrics     = std::string;
using Year       = uint;
using Disc       = uint;
using Track      = uint;
using Properties = std::unordered_map<std::string, std::string>;

using TitleCStr  = const char*;
using AlbumCStr  = const char*;
using ArtistCStr = const char*;
using GenreCStr  = const char*;
using LyricsCStr = const char*;

using Artists = std::vector<Artist>;
using Albums  = std::vector<Album>;
using Genres  = std::vector<Genre>;

template <typename T, typename S> using BucketedMap = std::map<T, std::vector<S>>;

/**
 * @brief A structure to hold metadata information for a song.
 */
struct Metadata
{
  Title       title                = "<Unknown Title>";
  Artist      artist               = "<Unknown Artist>";
  Album       album                = "<Unknown Album>";
  Genre       genre                = "<Unknown Genre>";
  std::string comment              = "<No Comment>";
  Year        year                 = 0;
  Track       track                = 0;
  uint        trackTotal           = 0;
  Disc        discNumber           = 0;
  uint        discTotal            = 0;
  Lyrics      lyrics               = "<No Lyrics>";
  Properties  additionalProperties = {};
  PathStr     filePath             = "";
  float       duration             = 0.0f;
  int         bitrate              = 0;

  std::string artUrl = "";

  template <class Archive> void serialize(Archive& ar)
  {
    ar(title, artist, album, genre, comment, year, track, trackTotal, discNumber, discTotal, lyrics,
       additionalProperties, filePath, duration, bitrate, artUrl);
  }
};

// ============================================================
// Song Structure
// ============================================================
struct Song
{
  ino_t    inode;    /**< The inode of the file representing the song */
  Metadata metadata; /**< Metadata information for the song */

  Song(ino_t inode, Metadata metadata) : inode(inode), metadata(std::move(metadata)) {};
  Song() : inode(0), metadata() {};
  explicit Song(ino_t inode) : inode(inode) {}

  template <class Archive> void serialize(Archive& ar) { ar(inode, metadata); }
};

//
// NOTE:
//
// If you have the entire song map, use the inode map (that is NOT in the Song structure)
//
// If you only have the Song struct at your disposal (say for printing, etc.) just Song.inode
// instead.
//
// Theoretically there should be no difference between the two but this can be a bit confusing.
//
// This was initially done to account for duplicated metadata files.
//
using Songs = std::vector<Song>;

using InodeMap = std::map<ino_t, Song>;
using TrackMap = std::map<Track, InodeMap>;
using DiscMap  = std::map<Disc, TrackMap>;
using AlbumMap = std::map<Album, DiscMap>;

// this corresponds to Artist -> Album -> Disc -> Track -> Inode -> Song
using SongMap = std::map<Artist, AlbumMap>;
