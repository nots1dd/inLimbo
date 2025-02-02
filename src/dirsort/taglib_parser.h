/**
 * @file taglib_parser.h
 * @brief A header file for the TagLibParser class and Metadata structure, used to parse metadata from audio files.
 *
 * This file defines a `Metadata` structure for storing song information such as title, artist, album, and more. It also defines the `TagLibParser` class for reading and extracting metadata from audio files using the TagLib library.
 * In addition, the file provides utility functions for printing metadata, handling error messages, and extracting thumbnails from audio files.
 *
 * **Note**: In the Emscripten build, dummy functions are used because TagLib must be compiled using Emscripten to function correctly in a web environment.
 */
#pragma once 

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
  std::string artist = "<Unknown Artist>"; /**< The artist of the song */
  std::string album = "Unknown Album"; /**< The album the song is part of */
  std::string genre = "Unknown Genre"; /**< The genre of the song */
  std::string comment = "No Comment"; /**< The comment associated with the song */
  std::string fileType = "NULL";
  unsigned int year = 0; /**< The year of release */
  unsigned int track = 0; /**< The track number */
  unsigned int discNumber = 0; /**< The disc number in a multi-disc set */
  std::string lyrics = "No Lyrics"; /**< The lyrics of the song */
  std::unordered_map<std::string, std::string> additionalProperties; /**< Any additional properties from the song's metadata */
  std::string filePath; /**< The file path of the song */
  float duration = 0.0f; /**< The duration of the song in seconds */
  int bitrate = 0; /**< The bitrate of the song (not calculated) */

  /**
   * @brief Serialization function for Metadata.
   * @param ar The archive used for serialization.
   */
  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(title, artist, album, genre, comment, year, track, discNumber, lyrics, additionalProperties, filePath, duration, bitrate);
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
  auto parseFile(const std::string& filePath, Metadata& metadata) -> bool;

  /**
   * @brief Parse metadata from files in a directory based on inode.
   * @param inode The inode of the file to search for.
   * @param directory The directory to search in.
   * @return A map of file paths to corresponding metadata.
   */
  auto parseFromInode(ino_t inode, const std::string& directory) -> std::unordered_map<std::string, Metadata>;
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

bool debugLogBool;

int unknownArtistTracks = 0;

// Constructor with parameter
TagLibParser::TagLibParser(const std::string& debugString) { debugLogBoolStr = debugString; debugLogBool = debugLogBoolStr == "true" ? true : false; }

void sendErrMsg(std::string debugLogBoolStr, std::string errMsg)
{
  
  if (debugLogBool)
  {
    std::cerr << errMsg << std::endl;
  }

  return;
}

#ifndef __EMSCRIPTEN__ // TagLib-specific implementations

// Function to parse metadata from a file
auto TagLibParser::parseFile(const std::string& filePath, Metadata& metadata) -> bool {
  if (debugLogBool) {
    std::cout << "-- [TAG PARSE] Parsing file: " << filePath << std::endl;
  }

  TagLib::FileRef file(filePath.c_str());
  std::string errMsg;
  
  // If file is invalid or cannot be opened
  if (file.isNull()) {
    errMsg = "Error: Failed to open file: " + filePath;
    sendErrMsg(debugLogBoolStr, errMsg);
    if (debugLogBool) {
      std::cout << "[TAG PARSE] " << errMsg << std::endl;
    }
    metadata.title = filePath.substr(filePath.find_last_of("/\\") + 1); // Use the file name 
    return false;
  }

  // If file has no tag information
  if (!file.tag()) {
    errMsg = "Error: No tag information found in file: " + filePath;
    sendErrMsg(debugLogBoolStr, errMsg);
    if (debugLogBool) {
      std::cout << "[TAG PARSE] " << errMsg << std::endl;
    }

    // Fallback metadata in case there is no tag information
    metadata.title = filePath.substr(filePath.find_last_of("/\\") + 1); // Use the file name 
    return true;
  }

  // If the file contains tag data, continue processing
  TagLib::Tag* tag = file.tag();
  metadata.title = tag->title().isEmpty() ? filePath.substr(filePath.find_last_of("/\\") + 1) : tag->title().to8Bit(true);
  metadata.artist = tag->artist().isEmpty() ? "<Unknown Artist>" : tag->artist().to8Bit(true);
  metadata.album = tag->album().isEmpty() ? "Unknown Album" : tag->album().to8Bit(true);
  metadata.genre = tag->genre().isEmpty() ? "Unknown Genre" : tag->genre().to8Bit(true);
  metadata.comment = tag->comment().isEmpty() ? "No Comment" : tag->comment().to8Bit(true);
  metadata.year = tag->year() == 0 ? 0 : tag->year();

  // Track logic
  metadata.track = tag->track();
  if (debugLogBool) {
    std::cout << "[TAG PARSE] Track: " << (tag->track() == 0 ? "(No Track, using fallback)" : std::to_string(tag->track())) << std::endl;
    
  }

  if (tag->track() == 0) {
    if (metadata.artist == "<Unknown Artist>") {
      unknownArtistTracks++;
      metadata.track = unknownArtistTracks;
    }
  }
  if (debugLogBool) std::cout << "[TAG PARSE]  Assigned track number: " << metadata.track << std::endl;

  // Audio Properties
  TagLib::AudioProperties *audioProperties = file.audioProperties();
  if (audioProperties) {
    metadata.duration = audioProperties->length(); // Duration in seconds
    metadata.bitrate = (audioProperties->bitrate() == 0) ? -1 : audioProperties->bitrate();

    if (debugLogBool) {
      std::cout << "[TAG PARSE] Audio properties found:" << std::endl;
      std::cout << "[TAG PARSE]  Duration: " << metadata.duration << " seconds" << std::endl;
      std::cout << "[TAG PARSE]  Bitrate: " << metadata.bitrate << " kbps" << std::endl;
    }
  } else {
    if (debugLogBool) {
      std::cout << "[TAG PARSE] No audio properties found." << std::endl;
    }
    metadata.duration = 0.0f;
    metadata.bitrate = 0;
  }

  // Keep track of file path
  metadata.filePath = filePath;
  if (debugLogBool) {
    std::cout << "[TAG PARSE] File Path: " << metadata.filePath << std::endl;
  }

  // Extract additional properties such as lyrics and disc number
  TagLib::PropertyMap properties = file.file()->properties();
  if (properties.contains("DISCNUMBER")) {
    metadata.discNumber = properties["DISCNUMBER"].toString().toInt();
    if (debugLogBool) {
      std::cout << "[TAG PARSE] Disc Number: " << metadata.discNumber << std::endl;
    }
  } else {
    metadata.discNumber = 0;
    if (debugLogBool) {
      std::cout << "[TAG PARSE] Disc Number: (Not Available)" << std::endl;
    }
  }

  if (properties.contains("LYRICS")) {
    metadata.lyrics = properties["LYRICS"].toString().to8Bit(true);
  } else {
    metadata.lyrics = "No Lyrics";
    if (debugLogBool) {
      std::cout << "[TAG PARSE] Lyrics: (Not Available)" << std::endl;
    }
  }

  // Populate additional properties if needed
  if (debugLogBool && !properties.isEmpty()) {
    std::cout << "[TAG PARSE] Additional properties found!" << std::endl;
    for (const auto& prop : properties) {
      std::string key = prop.first.to8Bit(true);
      std::string value = prop.second.toString().to8Bit(true);
      metadata.additionalProperties[key] = value;
    }
  }

  return true;
}

// Function to parse metadata based on inode
auto TagLibParser::parseFromInode(ino_t inode, const std::string& directory) -> std::unordered_map<std::string, Metadata> {
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
auto extractThumbnail(const std::string& audioFilePath, const std::string& outputImagePath) -> bool {
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
