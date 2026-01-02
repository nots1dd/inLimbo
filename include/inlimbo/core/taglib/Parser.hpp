#pragma once

#include "../MetadataTypes.hpp"
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <unordered_map>

#include <png.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace fs = std::filesystem;

/**
 * @brief A structure to hold metadata information for a song.
 */
struct Metadata
{
  Title                                        title      = "<Unknown Title>";
  Artist                                       artist     = "<Unknown Artist>";
  Album                                        album      = "<Unknown Album>";
  Genre                                        genre      = "<Unknown Genre>";
  std::string                                  comment    = "<No Comment>";
  Year                                         year       = 0;
  Track                                        track      = 0;
  Disc                                         discNumber = 0;
  Lyrics                                       lyrics     = "No Lyrics";
  std::unordered_map<std::string, std::string> additionalProperties;
  std::string                                  filePath;
  float                                        duration = 0.0f;
  int                                          bitrate  = 0;

  template <class Archive> void serialize(Archive& ar)
  {
    ar(title, artist, album, genre, comment, year, track, discNumber, lyrics, additionalProperties,
       filePath, duration, bitrate);
  }
};

/**
 * @brief A class for parsing metadata from audio files using TagLib.
 */
class TagLibParser
{
public:
  explicit TagLibParser(const std::string& debugString);

  auto parseFile(const std::string& filePath, Metadata& metadata) -> bool;
  auto parseFromInode(ino_t inode, const std::string& directory)
    -> std::unordered_map<std::string, Metadata>;

  auto modifyMetadata(const std::string& filePath, const Metadata& newData) -> bool;

private:
  void sendErrMsg(const std::string& errMsg);
  bool debugLogBool = false;
};

/**
 * @brief Extracts album art (thumbnail) from audio files (MP3, FLAC).
 * @return true on success.
 */
auto extractThumbnail(const std::string& audioFilePath, const std::string& outputImagePath) -> bool;
