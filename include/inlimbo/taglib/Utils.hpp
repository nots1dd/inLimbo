#pragma once

#include "InLimbo-Types.hpp"
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

namespace taglib::utils
{

auto parseFractionField(const TagLib::String& text) -> std::pair<int, int>;
void extractTrackAndTotal(TagLib::FileRef& file, Metadata& metadata);
void extractDiscAndTotal(TagLib::FileRef& file, Metadata& metadata);
void extractAudioProperties(TagLib::FileRef& file, Metadata& metadata);

} // namespace taglib::utils
