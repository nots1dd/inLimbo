#include "taglib/source/FLAC.hpp"
#include "taglib/Properties.hpp"
#include "taglib/source/Common.hpp"
#include <fstream>
#include <taglib/flacfile.h>

namespace taglib::source
{

auto FLAC::parse(const Path& filePath, Metadata& metadata, TagLibConfig&,
                 ParseSession& parseSession) -> bool
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

auto FLAC::modify(const Path& filePath, const Metadata& newData, TagLibConfig&) -> bool
{
  TagLib::FLAC::File file(filePath.c_str());
  if (!file.isValid())
    return false;

  TagLib::Tag* tag = file.tag();
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

auto FLAC::extractThumbnail(const Path& audioFilePath, const Path& outputImagePath) -> bool
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

} // namespace taglib::source
