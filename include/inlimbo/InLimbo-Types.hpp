#pragma once

#include "utils/ankerl/Cereal.hpp"
#include "utils/string/SmallString.hpp"
#include <cstdint>
#include <string>
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

using strvec  = std::vector<std::string>;
using Floats  = std::vector<float>;
using Doubles = std::vector<double>;
using Ints    = std::vector<int>;
using Bytes   = std::vector<std::byte>;

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
using Comment    = std::string;
using Year       = uint;
using Disc       = uint;
using Track      = uint;
using Properties = ankerl::unordered_dense::map<std::string, std::string>;

using TitleCStr  = const char*;
using AlbumCStr  = const char*;
using ArtistCStr = const char*;
using GenreCStr  = const char*;
using LyricsCStr = const char*;

using Artists = std::vector<Artist>;
using Albums  = std::vector<Album>;
using Genres  = std::vector<Genre>;

template <typename T, typename S>
using BucketedMap = ankerl::unordered_dense::map<T, std::vector<S>>;

/**
 * @brief A structure to hold metadata information for a song.
 */
struct Metadata
{
  Title      title;
  Artist     artist;
  Album      album;
  Genre      genre;
  Comment    comment;
  Year       year       = 0;
  Track      track      = 0;
  uint       trackTotal = 0;
  Disc       discNumber = 0;
  uint       discTotal  = 0;
  Lyrics     lyrics;
  Properties additionalProperties;
  PathStr    filePath;
  float      duration = 0.0f;
  int        bitrate  = 0;

  PathStr artUrl;

  template <class Archive>
  void serialize(Archive& ar)
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

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(inode, metadata);
  }
};

using Songs = std::vector<Song>;

// So we could use vector in case of Track and Disc mapping for better
// cache behaviour, but using ankerl here already is great in terms
// of memory usage and removing unneeded RBT allocs happening in std::map
//
// lookup time also should decrease but no benchmarks are done yet.
using InodeMap = ankerl::unordered_dense::map<ino_t, Song>;
using TrackMap = ankerl::unordered_dense::map<Track, InodeMap>;
using DiscMap  = ankerl::unordered_dense::map<Disc, TrackMap>;
using AlbumMap = ankerl::unordered_dense::map<Album, DiscMap>;

// this corresponds to Artist -> Album -> Disc -> Track -> Inode -> Song
using SongMap = ankerl::unordered_dense::map<Artist, AlbumMap>;
