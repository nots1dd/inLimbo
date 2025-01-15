#ifndef TAGLIB_PARSER_H
#define TAGLIB_PARSER_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#ifndef __EMSCRIPTEN__
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/tpropertymap.h>
#include <png.h>
#endif
#include <unordered_map>

namespace fs = std::filesystem;

// Metadata structure
struct Metadata
{
  std::string                                  title      = "Unknown Title";
  std::string                                  artist     = "Unknown Artist";
  std::string                                  album      = "Unknown Album";
  std::string                                  genre      = "Unknown Genre";
  std::string                                  comment    = "No Comment";
  unsigned int                                 year       = 0;
  unsigned int                                 track      = 0;
  unsigned int                                 discNumber = 0;
  std::string                                  lyrics     = "No Lyrics";
  std::unordered_map<std::string, std::string> additionalProperties;
  std::string                                  filePath;
  float duration = 0.0f;
};

// TagLibParser class for parsing metadata
class TagLibParser
{
public:
  explicit TagLibParser(const std::string& debugString);

  bool parseFile(const std::string& filePath, Metadata& metadata);

  std::unordered_map<std::string, Metadata> parseFromInode(ino_t              inode,
                                                           const std::string& directory);
};

// Function to print metadata
void printMetadata(const Metadata& metadata);

void sendErrMsg(std::string debugLogBoolStr, std::string errMsg);

// variable to keep track of user's preference of debug logs
std::string debugLogBoolStr;

// Constructor with parameter
TagLibParser::TagLibParser(const std::string& debugString) { debugLogBoolStr = debugString; }

void sendErrMsg(std::string debugLogBoolStr, std::string errMsg)
{
  if (debugLogBoolStr == "false")
  {
    return;
  }

  else if (debugLogBoolStr == "true")
  {
    std::cerr << errMsg << std::endl;
  }
  else
  {
    std::cerr << "invalid field in config.toml: " << debugLogBoolStr << std::endl;
  }

  return;
}

#ifndef __EMSCRIPTEN__ // TagLib-specific implementations

// Function to parse metadata from a file
bool TagLibParser::parseFile(const std::string& filePath, Metadata& metadata) {
  TagLib::FileRef file(filePath.c_str());
  std::string     errMsg;
  if (file.isNull()) {
    errMsg = "Error: Failed to open file: " + filePath;
    sendErrMsg(debugLogBoolStr, errMsg);
    return false;
  }

  if (!file.tag()) {
    errMsg = "Error: No tag information found in file: " + filePath;
    sendErrMsg(debugLogBoolStr, errMsg);
    return false;
  }

  TagLib::Tag* tag = file.tag();
  metadata.title   = tag->title().isEmpty() ? "Unknown Title" : tag->title().to8Bit(true);
  metadata.artist  = tag->artist().isEmpty() ? "Unknown Artist" : tag->artist().to8Bit(true);
  metadata.album   = tag->album().isEmpty() ? "Unknown Album" : tag->album().to8Bit(true);
  metadata.genre   = tag->genre().isEmpty() ? "Unknown Genre" : tag->genre().to8Bit(true);
  metadata.comment = tag->comment().isEmpty() ? "No Comment" : tag->comment().to8Bit(true);
  metadata.year    = tag->year() == 0 ? 0 : tag->year();
  metadata.track   = tag->track() == 0 ? 0 : tag->track();
  
  TagLib::AudioProperties *audioProperties = file.audioProperties();
  if (audioProperties) {
    metadata.duration = audioProperties->length(); // Duration in seconds
  }

  // Keep track of file path
  metadata.filePath = filePath;

  // Extract additional properties such as lyrics and disc number
  TagLib::PropertyMap properties = file.file()->properties();
  if (properties.contains("DISCNUMBER")) {
    metadata.discNumber = properties["DISCNUMBER"].toString().toInt();
  } else {
    metadata.discNumber = 0;
  }

  if (properties.contains("LYRICS")) {
    metadata.lyrics = properties["LYRICS"].toString().to8Bit(true);
  } else {
    metadata.lyrics = "No Lyrics";
  }

  // Populate additional properties if needed
  for (const auto& prop : properties) {
    std::string key                    = prop.first.to8Bit(true);
    std::string value                  = prop.second.toString().to8Bit(true);
    metadata.additionalProperties[key] = value;
  }

  return true;
}

// Function to parse metadata based on inode
std::unordered_map<std::string, Metadata> TagLibParser::parseFromInode(ino_t inode, const std::string& directory) {
  std::unordered_map<std::string, Metadata> metadataMap;
  std::string                               tempErrMsg;

  for (const auto& entry : fs::recursive_directory_iterator(directory)) {
    struct stat fileStat;
    if (stat(entry.path().c_str(), &fileStat) == 0) {
      if (fileStat.st_ino == inode) {
        Metadata metadata;
        if (parseFile(entry.path().string(), metadata)) {
          metadataMap[entry.path().string()] = metadata;
        } else {
          tempErrMsg = "Error: Unable to parse metadata for file: " + entry.path().string();
          sendErrMsg(debugLogBoolStr, tempErrMsg);
        }
      }
    } else {
      tempErrMsg = "Error: Unable to stat file: " + entry.path().string();
      sendErrMsg(debugLogBoolStr, tempErrMsg);
    }
  }

  return metadataMap;
}

#else // Stub implementations for Emscripten

bool TagLibParser::parseFile(const std::string& filePath, Metadata& metadata) {
  sendErrMsg(debugLogBoolStr, "TagLib is not available in this build.");
  return false;
}

std::unordered_map<std::string, Metadata> TagLibParser::parseFromInode(ino_t inode, const std::string& directory) {
  sendErrMsg(debugLogBoolStr, "TagLib is not available in this build.");
  return {};
}

#endif // __EMSCRIPTEN__

// Function to print metadata
void printMetadata(const Metadata& metadata) {
  std::cout << "Title: " << metadata.title << std::endl;
  std::cout << "Artist: " << metadata.artist << std::endl;
  std::cout << "Album: " << metadata.album << std::endl;
  std::cout << "Genre: " << metadata.genre << std::endl;
  std::cout << "Comment: " << metadata.comment << std::endl;
  std::cout << "Year: " << metadata.year << std::endl;
  std::cout << "Track: " << metadata.track << std::endl;
  std::cout << "Disc Number: " << metadata.discNumber << std::endl;
  std::cout << "Lyrics: " << std::endl << metadata.lyrics << std::endl;
  std::cout << "+++++++++++++++++++++++++++" << std::endl;
}

bool extractThumbnail(const std::string& audioFilePath, const std::string& outputImagePath) {
    // Open the file using TagLib
    TagLib::MPEG::File file(audioFilePath.c_str());
    if (!file.isValid()) {
        std::cerr << "Error: Could not open audio file." << std::endl;
        return false;
    }

    // Ensure the file has ID3v2 tags
    TagLib::ID3v2::Tag* id3v2Tag = file.ID3v2Tag();
    if (!id3v2Tag) {
        std::cerr << "Error: No ID3v2 tags found in the audio file." << std::endl;
        return false;
    }

    // Search for the APIC (Attached Picture) frame
    const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameListMap()["APIC"];
    if (frameList.isEmpty()) {
        std::cerr << "Error: No embedded album art found in the audio file." << std::endl;
        return false;
    }

    // Extract the first APIC frame (album art)
    auto* apicFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
    if (!apicFrame) {
        std::cerr << "Error: Failed to retrieve album art." << std::endl;
        return false;
    }

    // Get the picture data and MIME type
    const auto& pictureData = apicFrame->picture();
    const std::string mimeType = apicFrame->mimeType().toCString(true);

    // Save the picture data to a file
    std::ofstream outputFile(outputImagePath, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Could not create output image file." << std::endl;
        return false;
    }

    outputFile.write(reinterpret_cast<const char*>(pictureData.data()), pictureData.size());
    outputFile.close();

    return true;
}

#endif // TAGLIB_PARSER_H
