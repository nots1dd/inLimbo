#pragma once

#include "InLimbo-Types.hpp"

#define INLIMBO_TITLE_NAME_FALLBACK  "<Unknown-Title>"
#define INLIMBO_ARTIST_NAME_FALLBACK "<Unknown-Artist>"
#define INLIMBO_ALBUM_NAME_FALLBACK  "<Unknown-Album>"
#define INLIMBO_GENRE_NAME_FALLBACK  "<Unknown-Genre>"
#define INLIMBO_COMMENT_FALLBACK     "<No-Comment>"

namespace taglib
{

struct TagLibConfig
{
  bool debugLog = false;
};

struct ParseSession
{
  int unknownArtistTracks = 0;
};

// a generic parent interface class
// for audio types' (like MP3, FLAC, OGG, etc.)
// respective classes to inherit and call their
// custom parse, modify, extract thumbnail, etc.
//
// We construct a source table at compile time of
// all sources (currently only MP3 and FLAC)

class ITag
{
public:
  virtual ~ITag()                                                                  = default;
  virtual auto parse(const Path&, Metadata&, TagLibConfig&, ParseSession&) -> bool = 0;
  virtual auto modify(const Path&, const Metadata&, TagLibConfig&) -> bool         = 0;
  virtual auto extractThumbnail(const Path&, const Path&) -> bool                  = 0;
};

struct TagSourceEntry
{
  std::string_view ext;
  ITag*            source_tag;
};

} // namespace taglib
