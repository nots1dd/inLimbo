#pragma once

#include "InLimbo-Types.hpp"
#include <filesystem>

namespace utils::fs
{

auto toAbsFilePathUri(const std::filesystem::path& p) -> const Path
{
  Path uri("file://");

  uri += std::filesystem::absolute(p.c_str()).string();

  return uri;
}

auto fromAbsFilePathUri(const std::string uriPath) -> const Path
{
  Path path;

  if (uriPath.starts_with("file://"))
  {
    path += uriPath.substr(7);
  }

  return path;
}

} // namespace utils::fs
