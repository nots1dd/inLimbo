#pragma once

#include "InLimbo-Types.hpp"
#include "core/SongTree.hpp"
#include "taglib/Parser.hpp"

namespace helpers::fs
{

void dirWalkProcessAll(const Directory& directory, taglib::Parser& parser,
                       core::SongTree& songTree);

} // namespace helpers::fs
