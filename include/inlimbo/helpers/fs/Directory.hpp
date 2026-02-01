#pragma once

#include "InLimbo-Types.hpp"
#include "core/SongTree.hpp"
#include "taglib/Parser.hpp"
#include <filesystem>

namespace helpers::fs
{

auto toAbsFilePathUri(const std::filesystem::path& p) -> const Path;
auto fromAbsFilePathUri(const std::string uriPath) -> const Path;
void dirWalkProcessAll(const Directory& directory, taglib::Parser& parser,
                       core::SongTree& songTree);

} // namespace helpers::fs
