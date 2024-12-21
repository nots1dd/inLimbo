#include "taglib_parser.h"
#include <iostream>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <filesystem>
#include <sys/stat.h>

namespace fs = std::filesystem;

// Constructor
TagLibParser::TagLibParser() = default;

// Function to parse metadata from a file
bool TagLibParser::parseFile(const std::string &filePath, Metadata &metadata) {
    TagLib::FileRef file(filePath.c_str());
    if (file.isNull()) {
        std::cerr << "Error: Failed to open file: " << filePath << std::endl;
        return false;
    }

    if (!file.tag()) {
        std::cerr << "Error: No tag information found in file: " << filePath << std::endl;
        return false;
    }

    TagLib::Tag *tag = file.tag();
    metadata.title = tag->title().isEmpty() ? "Unknown Title" : tag->title().to8Bit(true);
    metadata.artist = tag->artist().isEmpty() ? "Unknown Artist" : tag->artist().to8Bit(true);
    metadata.album = tag->album().isEmpty() ? "Unknown Album" : tag->album().to8Bit(true);
    metadata.genre = tag->genre().isEmpty() ? "Unknown Genre" : tag->genre().to8Bit(true);
    metadata.comment = tag->comment().isEmpty() ? "No Comment" : tag->comment().to8Bit(true);
    metadata.year = tag->year() == 0 ? 0 : tag->year();
    metadata.track = tag->track() == 0 ? 0 : tag->track();

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
    for (const auto &prop : properties) {
        std::string key = prop.first.to8Bit(true);
        std::string value = prop.second.toString().to8Bit(true);
        metadata.additionalProperties[key] = value;
    }

    return true;
}

// Function to parse metadata based on inode
std::unordered_map<std::string, Metadata> TagLibParser::parseFromInode(ino_t inode, const std::string &directory) {
    std::unordered_map<std::string, Metadata> metadataMap;

    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        struct stat fileStat;
        if (stat(entry.path().c_str(), &fileStat) == 0) {
            if (fileStat.st_ino == inode) {
                Metadata metadata;
                if (parseFile(entry.path().string(), metadata)) {
                    metadataMap[entry.path().string()] = metadata;
                } else {
                    std::cerr << "Error: Unable to parse metadata for file: " << entry.path().string() << std::endl;
                }
            }
        } else {
            std::cerr << "Error: Unable to stat file: " << entry.path().string() << " (" << errno << ")" << std::endl;
        }
    }

    return metadataMap;
}

// Function to print metadata
void printMetadata(const Metadata &metadata) {
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
