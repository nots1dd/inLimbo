#pragma once

#include "ITag.hpp"
#include "taglib/source/FLAC.hpp"
#include "taglib/source/MP3.hpp"
#include <sys/stat.h>

#include <png.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace taglib
{

class Parser
{
public:
  explicit Parser(TagLibConfig config);

  auto parseFile(const Path& filePath, Metadata& metadata) -> bool;
  auto modifyMetadata(const Path& filePath, const Metadata& newData) -> bool;

  static auto fillArtUrl(Metadata& meta) -> bool;

private:
  TagLibConfig m_config;
  ParseSession m_parseSession;
};

static source::MP3  MP3_STRATEGY;
static source::FLAC FLAC_STRATEGY;

constexpr std::array SOURCE_TABLE = {
  TagSourceEntry{.ext = ".mp3", .source_tag = &MP3_STRATEGY},
  TagSourceEntry{.ext = ".flac", .source_tag = &FLAC_STRATEGY},
};

inline static auto findSource(std::string_view ext) -> ITag*
{
  for (auto& s : SOURCE_TABLE)
    if (s.ext == ext)
      return s.source_tag;
  return nullptr;
}

} // namespace taglib
