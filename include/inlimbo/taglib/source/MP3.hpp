#pragma once

#include "taglib/ITag.hpp"
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

namespace taglib::source
{

class MP3 final : public ITag
{
public:
  auto parse(const Path& filePath, Metadata& metadata, TagLibConfig&, ParseSession& parseSession)
    -> bool override;
  auto modify(const Path& filePath, const Metadata& newData, TagLibConfig&) -> bool override;
  auto extractThumbnail(const Path& audioFilePath, const Path& outputImagePath) -> bool override;
};

} // namespace taglib::source
