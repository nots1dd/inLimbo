#pragma once

#include "InLimbo-Types.hpp"
#include "core/SongLibrarySnapshot.hpp"
#include "taglib/Parser.hpp"

namespace helpers::fs
{

void dirWalkProcessAll(const Directory& directory, taglib::Parser& tagParser,
                       core::SongLibrarySnapshot& songLibrarySnapshot);

} // namespace helpers::fs
