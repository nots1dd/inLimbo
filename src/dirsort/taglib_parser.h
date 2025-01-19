/**
 * @file taglib_parser.h
 * @brief A header file for the TagLibParser class and Metadata structure, used to parse metadata from audio files.
 *
 * This file defines a `Metadata` structure for storing song information such as title, artist, album, and more. It also defines the `TagLibParser` class for reading and extracting metadata from audio files using the TagLib library.
 * In addition, the file provides utility functions for printing metadata, handling error messages, and extracting thumbnails from audio files.
 *
 * **Note**: In the Emscripten build, dummy functions are used because TagLib must be compiled using Emscripten to function correctly in a web environment.
 */
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
#include <taglib/flacfile.h>
#include <png.h>
#endif
#include <unordered_map>

namespace fs = std::filesystem;

/**
 * @brief A structure to hold metadata information for a song.
 *
 * This structure contains various attributes related to a song, such as title, artist, album, genre, and more.
 * It also allows additional properties and file path to be stored, along with the song's duration.
 */

struct Metadata
{
  std::string title = "Unknown Title"; /**< The title of the song */
  std::string artist = "Unknown Artist"; /**< The artist of the song */
  std::string album = "Unknown Album"; /**< The album the song is part of */
  std::string genre = "Unknown Genre"; /**< The genre of the song */
  std::string comment = "No Comment"; /**< The comment associated with the song */
  unsigned int year = 0; /**< The year of release */
  unsigned int track = 0; /**< The track number */
  unsigned int discNumber = 0; /**< The disc number in a multi-disc set */
  std::string lyrics = "No Lyrics"; /**< The lyrics of the song */
  std::unordered_map<std::string, std::string> additionalProperties; /**< Any additional properties from the song's metadata */
  std::string filePath; /**< The file path of the song */
  float duration = 0.0f; /**< The duration of the song in seconds */

  /**
   * @brief Serialization function for Metadata.
   * @param ar The archive used for serialization.
   */
  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(title, artist, album, genre, comment, year, track, discNumber, lyrics, additionalProperties, filePath, duration);
  }
};

/**
 * @brief A class for parsing metadata from audio files.
 *
 * The `TagLibParser` class uses the TagLib library to extract metadata from audio files.
 * It supports parsing tags, including title, artist, album, genre, year, and track information.
 * Additionally, it can parse metadata from files using inode information.
 */
class TagLibParser
{
public:
  /**
   * @brief Constructor for TagLibParser.
   * @param debugString A string to enable or disable debug logs.
   */
  explicit TagLibParser(const std::string& debugString);

  /**
   * @brief Parse metadata from an audio file.
   * @param filePath The path to the audio file.
   * @param metadata A reference to a Metadata object where parsed data will be stored.
   * @return `true` if parsing was successful, `false` otherwise.
   */
  bool parseFile(const std::string& filePath, Metadata& metadata);

  /**
   * @brief Parse metadata from files in a directory based on inode.
   * @param inode The inode of the file to search for.
   * @param directory The directory to search in.
   * @return A map of file paths to corresponding metadata.
   */
  std::unordered_map<std::string, Metadata> parseFromInode(ino_t              inode,
                                                           const std::string& directory);
};

/**
 * @brief Prints the metadata of a song to the console.
 * @param metadata The metadata of the song to print.
 */
void printMetadata(const Metadata& metadata);

/**
 * @brief Sends an error message based on the debug log setting.
 * @param debugLogBoolStr The debug log setting ("true" or "false").
 * @param errMsg The error message to send.
 */
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

/**
 * @brief Dummy implementation for `parseFile` when compiled with Emscripten.
 * @param filePath The path to the audio file.
 * @param metadata A reference to a Metadata object (not populated in this dummy implementation).
 * @return Always returns `false` as TagLib is not available.
 */
bool TagLibParser::parseFile(const std::string& filePath, Metadata& metadata) {
  sendErrMsg(debugLogBoolStr, "TagLib is not available in this build.");
  return false;
}

/**
 * @brief Dummy implementation for `parseFromInode` when compiled with Emscripten.
 * @param inode The inode of the file to search for.
 * @param directory The directory to search in.
 * @return Always returns an empty map as TagLib is not available.
 */
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

/**
 * @brief Extracts the thumbnail (album art) from an audio file and saves it to an image file. (Works for mp3 and flac)
 * @param audioFilePath The path to the audio file containing embedded album art.
 * @param outputImagePath The path where the extracted album art image will be saved.
 * @return `true` if the thumbnail was successfully extracted, `false` otherwise.
 */
bool extractThumbnail(const std::string& audioFilePath, const std::string& outputImagePath) {
    // Determine the file type based on the extension
    std::string extension = audioFilePath.substr(audioFilePath.find_last_of('.') + 1);

    if (extension == "mp3") {
        // Handle MP3 files (ID3v2 tag)
        TagLib::MPEG::File mpegFile(audioFilePath.c_str());
        if (!mpegFile.isValid()) {
            std::cerr << "Error: Could not open MP3 file." << std::endl;
            return false;
        }

        TagLib::ID3v2::Tag* id3v2Tag = mpegFile.ID3v2Tag();
        if (!id3v2Tag) {
            std::cerr << "Error: No ID3v2 tags found in the MP3 file." << std::endl;
            return false;
        }

        const TagLib::ID3v2::FrameList& frameList = id3v2Tag->frameListMap()["APIC"];
        if (frameList.isEmpty()) {
            std::cerr << "Error: No embedded album art found in the MP3 file." << std::endl;
            return false;
        }

        auto* apicFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
        if (!apicFrame) {
            std::cerr << "Error: Failed to retrieve album art from MP3." << std::endl;
            return false;
        }

        const auto& pictureData = apicFrame->picture();
        std::ofstream outputFile(outputImagePath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error: Could not create output image file." << std::endl;
            return false;
        }

        outputFile.write(reinterpret_cast<const char*>(pictureData.data()), pictureData.size());
        outputFile.close();
    } else if (extension == "flac") {
        // Handle FLAC files
        TagLib::FLAC::File flacFile(audioFilePath.c_str(), true);
        if (!flacFile.isValid()) {
            std::cerr << "Error: Could not open FLAC file." << std::endl;
            return false;
        }

        const TagLib::List<TagLib::FLAC::Picture*>& pictureList = flacFile.pictureList();
        if (pictureList.isEmpty()) {
            std::cerr << "Error: No album art found in the FLAC file." << std::endl;
            return false;
        }

        const auto& pictureData = pictureList.front()->data();
        std::ofstream outputFile(outputImagePath, std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error: Could not create output image file." << std::endl;
            return false;
        }

        outputFile.write(reinterpret_cast<const char*>(pictureData.data()), pictureData.size());
        outputFile.close();
    } else {
        std::cerr << "Error: Unsupported file format. Only MP3 and FLAC are supported." << std::endl;
        return false;
    }

    return true;
}

#endif // TAGLIB_PARSER_H
