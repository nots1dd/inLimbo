#pragma once

#include "InLimbo-Types.hpp"
#include <sys/stat.h>

#include <png.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace core
{

struct TagLibConfig
{
  bool debugLog = false;
};

/**
 * @brief A class for parsing metadata from audio files using TagLib.
 */
class TagLibParser
{
public:
  explicit TagLibParser(TagLibConfig config);

  auto parseFile(const Path& filePath, Metadata& metadata) -> bool;

  auto modifyMetadata(const Path& filePath, const Metadata& newData) -> bool;

private:
  TagLibConfig m_config;
};

/**
 * @brief Extracts album art (thumbnail) from audio files (MP3, FLAC).
 * @return true on success.
 */
auto extractThumbnail(const Path& audioFilePath, const Path& outputImagePath) -> bool;

} // namespace core
