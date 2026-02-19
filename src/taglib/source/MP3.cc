#include "taglib/source/MP3.hpp"
#include "taglib/Properties.hpp"
#include "taglib/source/Common.hpp"
#include <fstream>
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>

namespace taglib::source
{

auto MP3::parse(const Path& filePath, Metadata& metadata, TagLibConfig&, ParseSession& parseSession)
  -> bool
{
  TagLib::FileRef file(filePath.c_str());
  if (file.isNull())
    return false;

  if (!file.tag())
    return true;

  TagLib::Tag* tag = file.tag();

  CommonTag::fillBasic(tag, filePath, metadata);
  CommonTag::fillTrackDisc(file, metadata);
  CommonTag::fillProperties(file, metadata, parseSession.unknownArtistTracks);

  return true;
}

auto MP3::modify(const Path& filePath, const Metadata& newData, TagLibConfig&) -> bool
{
  TagLib::MPEG::File file(filePath.c_str());
  if (!file.isValid())
    return false;

  auto* tag = file.ID3v2Tag(true);
  if (!tag)
    return false;

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
    auto props = file.properties();
    props.replace(propTagString(PropKey::Lyrics),
                  TagLib::StringList(TagLib::String(newData.lyrics, TagLib::String::UTF8)));
    file.setProperties(props);
  }

  return file.save();
}

auto MP3::extractThumbnail(const Path& audioFilePath, const Path& outputImagePath) -> bool
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

} // namespace taglib::source
