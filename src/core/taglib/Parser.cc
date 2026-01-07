#include "core/taglib/Parser.hpp"
#include "Logger.hpp"
#include "utils/PathResolve.hpp"
#include <taglib/id3v2.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace core
{

static void extractTrackAndTotal(TagLib::FileRef& file, Metadata& metadata)
{
  TagLib::PropertyMap props = file.file()->properties();

  metadata.track      = 0;
  metadata.trackTotal = 0;

  // -------------------------
  // TRACKNUMBER: "X" or "X/Y"
  // -------------------------
  if (props.contains("TRACKNUMBER"))
  {
    TagLib::String text  = props["TRACKNUMBER"].toString(); // e.g. "3/9"
    auto           parts = text.split('/');

    if (!parts.isEmpty())
      metadata.track = parts[0].toInt();

    if (parts.size() > 1)
      metadata.trackTotal = parts[1].toInt();
  }

  // -------------------------
  // Fallback: TRACKTOTAL / TOTALTRACKS
  // -------------------------
  if (metadata.trackTotal == 0)
  {
    if (props.contains("TRACKTOTAL"))
      metadata.trackTotal = props["TRACKTOTAL"].toString().toInt();
    else if (props.contains("TOTALTRACKS"))
      metadata.trackTotal = props["TOTALTRACKS"].toString().toInt();
  }
}

static void extractDiscAndTotal(TagLib::FileRef& file, Metadata& metadata)
{
  TagLib::PropertyMap props = file.file()->properties();

  metadata.discNumber = 0;
  metadata.discTotal  = 0;

  // -------------------------
  // DISCNUMBER: "X" or "X/Y"
  // -------------------------
  if (props.contains("DISCNUMBER"))
  {
    TagLib::String text  = props["DISCNUMBER"].toString(); // e.g. "1/2"
    auto           parts = text.split('/');

    if (!parts.isEmpty())
      metadata.discNumber = parts[0].toInt();

    if (parts.size() > 1)
      metadata.discTotal = parts[1].toInt();
  }

  // -------------------------
  // Fallback: DISCTOTAL / TOTALDISCS
  // -------------------------
  if (metadata.discTotal == 0)
  {
    if (props.contains("DISCTOTAL"))
      metadata.discTotal = props["DISCTOTAL"].toString().toInt();
    else if (props.contains("TOTALDISCS"))
      metadata.discTotal = props["TOTALDISCS"].toString().toInt();
  }
}

static int unknownArtistTracks = 0;

TagLibParser::TagLibParser(TagLibConfig config) { m_config = std::move(config); }

auto TagLibParser::parseFile(const Path& filePath, Metadata& metadata) -> bool
{
  if (m_config.debugLog)
    LOG_DEBUG("Parsing file: {}", filePath);

  TagLib::FileRef file(filePath.c_str());
  if (file.isNull())
  {
    LOG_ERROR("Failed to open file: {}", filePath);
    metadata.title = filePath.filename();
    return false;
  }

  if (!file.tag())
  {
    LOG_WARN("No tag information found in file: {}", filePath);
    metadata.title = filePath.filename();
    return true;
  }

  TagLib::Tag* tag = file.tag();
  metadata.title = tag->title().isEmpty() ? filePath.filename().c_str() : tag->title().to8Bit(true);
  metadata.artist     = tag->artist().isEmpty() ? "<Unknown Artist>" : tag->artist().to8Bit(true);
  metadata.album      = tag->album().isEmpty() ? "<Unknown Album>" : tag->album().to8Bit(true);
  metadata.genre      = tag->genre().isEmpty() ? "<Unknown Genre>" : tag->genre().to8Bit(true);
  metadata.comment    = tag->comment().isEmpty() ? "<No Comment>" : tag->comment().to8Bit(true);
  metadata.year       = tag->year();
  metadata.track      = tag->track();
  metadata.trackTotal = 0;

  extractDiscAndTotal(file, metadata);
  extractTrackAndTotal(file, metadata);

  TagLib::AudioProperties* audioProps = file.audioProperties();
  if (audioProps)
  {
    metadata.duration = audioProps->lengthInSeconds();
    metadata.bitrate  = audioProps->bitrate();
  }

  metadata.filePath = filePath;

  TagLib::PropertyMap props = file.file()->properties();

  // this is required to sort features in a song properly.
  //
  // if a song is indexed as <Song> feat. <Artist B> but it is in
  // the album of Artist A, the metadata in artist tag of TagLib
  // shows: <Artist A>/<Artist B>
  //
  // This isnt entirely wrong, it lets us know features and mixed songs.
  //
  // But when fetching albums which is very important, the song tree screws up.
  //
  // It finds a new key (<Artist A>/<Artist B>) and will use that as a "new" artist.
  // So the album although maybe downloaded by the user, it wont show up contiguosly.
  //
  // If we dont find this property "ALBUMARTIST", we have the fallback artist tag anyway.
  if (props.contains("ALBUMARTIST"))
    metadata.artist = props["ALBUMARTIST"].toString().to8Bit(true);

  if (metadata.track == 0 && metadata.artist == "<Unknown Artist>")
    metadata.track = ++unknownArtistTracks;

  if (props.contains("LYRICS"))
    metadata.lyrics = props["LYRICS"].toString().to8Bit(true);

  for (const auto& [key, val] : props)
  {
    metadata.additionalProperties[key.to8Bit(true)] = val.toString().to8Bit(true);
  }

  // Art URL

  if (!fillArtUrl(metadata))
  {
    LOG_WARN("No embedded art found for file: {}", filePath);
    metadata.artUrl = "";
  }

  return true;
}

// NOTE: This function currently supports only MP3 and FLAC files.
//
// Currently modifies the following fields:
// - Title
// - Artist
// - Album
// - Genre
// - Comment
// - Year
// - Track
// - Lyrics
auto TagLibParser::modifyMetadata(const Path& filePath, const Metadata& newData) -> bool
{
  const auto ext     = filePath.extension();
  bool       success = false;

  try
  {
    if (ext == ".mp3")
    {
      TagLib::MPEG::File file(filePath.c_str());
      if (!file.isValid())
      {
        LOG_ERROR("Invalid MP3 file: {}", filePath);
        return false;
      }

      TagLib::ID3v2::Tag* tag = file.ID3v2Tag(true);
      if (!tag)
      {
        LOG_WARN("Unable to get ID3v2 tag for: {}", filePath);
        return false;
      }

      if (!newData.title.empty())
        tag->setTitle(TagLib::String(newData.title, TagLib::String::UTF8));
      if (!newData.artist.empty())
        tag->setArtist(TagLib::String(newData.artist, TagLib::String::UTF8));
      if (!newData.album.empty())
        tag->setAlbum(TagLib::String(newData.album, TagLib::String::UTF8));
      if (!newData.genre.empty())
        tag->setGenre(TagLib::String(newData.genre, TagLib::String::UTF8));
      if (!newData.comment.empty())
        tag->setComment(TagLib::String(newData.comment, TagLib::String::UTF8));
      if (newData.year != 0)
        tag->setYear(newData.year);
      if (newData.track != 0)
        tag->setTrack(newData.track);

      if (!newData.lyrics.empty())
      {
        TagLib::PropertyMap props = file.properties();
        props.replace("LYRICS",
          TagLib::StringList(TagLib::String(newData.lyrics, TagLib::String::UTF8)));
        file.setProperties(props);
      }

      file.save();
      success = true;
    }
    else if (ext == ".flac")
    {
      TagLib::FLAC::File file(filePath.c_str());
      if (!file.isValid())
      {
        LOG_WARN("Invalid FLAC file: {}", filePath);
        return false;
      }

      TagLib::Tag* tag = file.tag();
      if (!tag)
      {
        LOG_WARN("Unable to get tag for FLAC file: {}", filePath);
        return false;
      }

      if (!newData.title.empty())
        tag->setTitle(TagLib::String(newData.title, TagLib::String::UTF8));
      if (!newData.artist.empty())
        tag->setArtist(TagLib::String(newData.artist, TagLib::String::UTF8));
      if (!newData.album.empty())
        tag->setAlbum(TagLib::String(newData.album, TagLib::String::UTF8));
      if (!newData.genre.empty())
        tag->setGenre(TagLib::String(newData.genre, TagLib::String::UTF8));
      if (!newData.comment.empty())
        tag->setComment(TagLib::String(newData.comment, TagLib::String::UTF8));
      if (newData.year != 0)
        tag->setYear(newData.year);
      if (newData.track != 0)
        tag->setTrack(newData.track);

      if (!newData.lyrics.empty())
      {
        TagLib::PropertyMap props = file.properties();
        props.replace("LYRICS",
          TagLib::StringList(TagLib::String(newData.lyrics, TagLib::String::UTF8)));
        file.setProperties(props);
      }

      file.save();
      success = true;
    }
    else
    {
      LOG_WARN("Unsupported file type for metadata modification: {}", ext);
      return false;
    }

    if (m_config.debugLog && success)
      LOG_INFO("Metadata updated successfully for: {}", filePath);
  }
  catch (const std::exception& e)
  {
    LOG_WARN("Exception during metadata update: {}", e.what());
    success = false;
  }

  return success;
}

static auto toFileUri(const std::filesystem::path& p) -> const Path
{
  utils::string::SmallString uri("file://");

  uri += std::filesystem::absolute(p.c_str()).string();

  return uri;
}

auto extractThumbnail(const std::string& audioFilePath, const std::string& outputImagePath) -> bool
{
  const std::string ext = std::filesystem::path(audioFilePath).extension().string();

  if (ext == ".mp3")
  {
    TagLib::MPEG::File file(audioFilePath.c_str());
    if (!file.isValid())
      return false;
    auto* tag = file.ID3v2Tag();
    if (!tag)
      return false;

    const auto& frames = tag->frameListMap()["APIC"];
    if (frames.isEmpty())
      return false;

    auto* pic = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
    if (!pic)
      return false;

    std::ofstream out(outputImagePath.c_str(), std::ios::binary);
    out.write(reinterpret_cast<cstr>(pic->picture().data()), pic->picture().size());
    return true;
  }
  else if (ext == ".flac")
  {
    TagLib::FLAC::File file(audioFilePath.c_str());
    if (!file.isValid())
      return false;
    auto pics = file.pictureList();
    if (pics.isEmpty())
      return false;

    std::ofstream out(outputImagePath.c_str(), std::ios::binary);
    out.write(reinterpret_cast<cstr>(pics.front()->data().data()), pics.front()->data().size());
    return true;
  }

  return false;
}

auto TagLibParser::fillArtUrl(Metadata& meta) -> bool
{
  namespace fs = std::filesystem;

  const auto cacheDir = utils::getCacheArtPath();
  fs::create_directories(cacheDir.c_str());

  const std::string hash   = std::to_string(std::hash<std::string>{}(meta.filePath.c_str()));
  fs::path          outImg = cacheDir / (hash + ".jpg");

  if (fs::exists(outImg.c_str()) || extractThumbnail(meta.filePath, outImg))
  {
    meta.artUrl = toFileUri(outImg);
    return true;
  }

  meta.artUrl.clear();
  return false;
}

} // namespace core
