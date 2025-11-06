#include "core/taglib/Parser.hpp"
#include <string>
#include <fstream>
#include <iostream>

static int unknownArtistTracks = 0;

TagLibParser::TagLibParser(const std::string& debugString) {
    debugLogBool = (debugString == "true");
}

void TagLibParser::sendErrMsg(const std::string& errMsg) {
    if (debugLogBool) {
        LOG_ERROR(errMsg);
    }
}

auto TagLibParser::parseFile(const std::string& filePath, Metadata& metadata) -> bool {
    if (debugLogBool)
        LOG_DEBUG("Parsing file: {}", filePath);

    TagLib::FileRef file(filePath.c_str());
    if (file.isNull()) {
        sendErrMsg("Failed to open file: " + filePath);
        metadata.title = fs::path(filePath).filename().string();
        return false;
    }

    if (!file.tag()) {
        sendErrMsg("No tag information found in file: " + filePath);
        metadata.title = fs::path(filePath).filename().string();
        return true;
    }

    TagLib::Tag* tag = file.tag();
    metadata.title  = tag->title().isEmpty() ? fs::path(filePath).filename().string() : tag->title().to8Bit(true);
    metadata.artist = tag->artist().isEmpty() ? "<Unknown Artist>" : tag->artist().to8Bit(true);
    metadata.album  = tag->album().isEmpty() ? "Unknown Album" : tag->album().to8Bit(true);
    metadata.genre  = tag->genre().isEmpty() ? "Unknown Genre" : tag->genre().to8Bit(true);
    metadata.comment = tag->comment().isEmpty() ? "No Comment" : tag->comment().to8Bit(true);
    metadata.year = tag->year();
    metadata.track = tag->track();

    if (metadata.track == 0 && metadata.artist == "<Unknown Artist>")
        metadata.track = ++unknownArtistTracks;

    TagLib::AudioProperties* audioProps = file.audioProperties();
    if (audioProps) {
        metadata.duration = audioProps->length();
        metadata.bitrate = audioProps->bitrate();
    }

    metadata.filePath = filePath;

    TagLib::PropertyMap props = file.file()->properties();
    if (props.contains("DISCNUMBER"))
        metadata.discNumber = props["DISCNUMBER"].toString().toInt();

    if (props.contains("LYRICS"))
        metadata.lyrics = props["LYRICS"].toString().to8Bit(true);

    for (const auto& [key, val] : props) {
        metadata.additionalProperties[key.to8Bit(true)] = val.toString().to8Bit(true);
    }

    return true;
}

auto TagLibParser::parseFromInode(ino_t inode, const std::string& directory) -> std::unordered_map<std::string, Metadata> {
    std::unordered_map<std::string, Metadata> result;

    for (const auto& entry : fs::recursive_directory_iterator(directory)) {
        struct stat fileStat{};
        if (stat(entry.path().c_str(), &fileStat) == 0 && fileStat.st_ino == inode) {
            Metadata m;
            if (parseFile(entry.path().string(), m))
                result[entry.path().string()] = m;
            else
                sendErrMsg("Unable to parse: " + entry.path().string());
        }
    }
    return result;
}

void printMetadata(const Metadata& metadata) {
    std::cout << "Title: " << metadata.title << "\n"
              << "Artist: " << metadata.artist << "\n"
              << "Album: " << metadata.album << "\n"
              << "Genre: " << metadata.genre << "\n"
              << "Comment: " << metadata.comment << "\n"
              << "Year: " << metadata.year << "\n"
              << "Track: " << metadata.track << "\n"
              << "Disc: " << metadata.discNumber << "\n"
              << "Lyrics: " << metadata.lyrics << "\n"
              << "Duration: " << metadata.duration << "s\n"
              << "Bitrate: " << metadata.bitrate << "kbps\n"
              << "----------------------------------------\n";
}

auto extractThumbnail(const std::string& audioFilePath, const std::string& outputImagePath) -> bool {
    std::string ext = fs::path(audioFilePath).extension().string();

    if (ext == ".mp3") {
        TagLib::MPEG::File file(audioFilePath.c_str());
        if (!file.isValid()) return false;
        auto* tag = file.ID3v2Tag();
        if (!tag) return false;

        const auto& frames = tag->frameListMap()["APIC"];
        if (frames.isEmpty()) return false;

        auto* pic = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
        if (!pic) return false;

        std::ofstream out(outputImagePath, std::ios::binary);
        out.write(reinterpret_cast<const char*>(pic->picture().data()), pic->picture().size());
        return true;
    }
    else if (ext == ".flac") {
        TagLib::FLAC::File file(audioFilePath.c_str());
        if (!file.isValid()) return false;
        auto pics = file.pictureList();
        if (pics.isEmpty()) return false;

        std::ofstream out(outputImagePath, std::ios::binary);
        out.write(reinterpret_cast<const char*>(pics.front()->data().data()), pics.front()->data().size());
        return true;
    }

    return false;
}
