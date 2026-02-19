#pragma once

#include "taglib/ITag.hpp"
#include "taglib/Properties.hpp"
#include "taglib/Utils.hpp"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace taglib::source
{

struct CommonTag
{
  static inline void fillBasic(TagLib::Tag* tag, const Path& filePath, Metadata& metadata)
  {
    metadata.title =
      tag->title().isEmpty() ? filePath.filename().c_str() : tag->title().to8Bit(true);

    metadata.artist =
      tag->artist().isEmpty() ? INLIMBO_ARTIST_NAME_FALLBACK : tag->artist().to8Bit(true);

    metadata.album =
      tag->album().isEmpty() ? INLIMBO_ALBUM_NAME_FALLBACK : tag->album().to8Bit(true);

    metadata.genre =
      tag->genre().isEmpty() ? INLIMBO_GENRE_NAME_FALLBACK : tag->genre().to8Bit(true);

    metadata.comment =
      tag->comment().isEmpty() ? INLIMBO_COMMENT_FALLBACK : tag->comment().to8Bit(true);

    metadata.year  = tag->year();
    metadata.track = tag->track();
  }

  static inline void fillTrackDisc(TagLib::FileRef& file, Metadata& metadata)
  {
    utils::extractDiscAndTotal(file, metadata);
    utils::extractTrackAndTotal(file, metadata);
  }

  static inline void fillProperties(TagLib::FileRef& file, Metadata& metadata,
                                    int& unknownArtistTracks)
  {
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
    if (hasProp(props, PropKey::AlbumArtist))
      metadata.artist = getProp(props, PropKey::AlbumArtist).to8Bit(true);

    if (metadata.track == 0 && metadata.artist == INLIMBO_ARTIST_NAME_FALLBACK)
      metadata.track = ++unknownArtistTracks;

    if (hasProp(props, PropKey::Lyrics))
      metadata.lyrics = getProp(props, PropKey::Lyrics).to8Bit(true);

    for (const auto& [key, val] : props)
      metadata.additionalProperties[key.to8Bit(true)] = val.toString().to8Bit(true);

    utils::extractAudioProperties(file, metadata);
  }
};

} // namespace taglib::source
