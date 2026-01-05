#include "core/taglib/Parser.hpp"
#include "Logger.hpp"
#include <fstream>
#include <iostream>

namespace core
{

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
  metadata.artist  = tag->artist().isEmpty() ? "<Unknown Artist>" : tag->artist().to8Bit(true);
  metadata.album   = tag->album().isEmpty() ? "<Unknown Album>" : tag->album().to8Bit(true);
  metadata.genre   = tag->genre().isEmpty() ? "<Unknown Genre>" : tag->genre().to8Bit(true);
  metadata.comment = tag->comment().isEmpty() ? "<No Comment>" : tag->comment().to8Bit(true);
  metadata.year    = tag->year();
  metadata.track   = tag->track();

  TagLib::AudioProperties* audioProps = file.audioProperties();
  if (audioProps)
  {
    metadata.duration = audioProps->lengthInSeconds();
    metadata.bitrate  = audioProps->bitrate();
  }

  metadata.filePath = filePath;

  TagLib::PropertyMap props = file.file()->properties();
  if (props.contains("DISCNUMBER"))
    metadata.discNumber = props["DISCNUMBER"].toString().toInt();

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

  return true;
}

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

auto extractThumbnail(const Path& audioFilePath, const Path& outputImagePath) -> bool
{
  const utils::string::SmallString ext = audioFilePath.extension();

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

} // namespace core
