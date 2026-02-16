#pragma once

#include "InLimbo-Types.hpp"
#include <filesystem>

namespace utils::fs
{

// Convert regular file path to a file URI. The resulting URI will have the format
// "file://path/to/file".
inline auto toRelFilePathUri(const std::filesystem::path& p) -> const Path
{
  Path uri("file://");

  uri += p.c_str();

  return uri;
}

// Convert regular file path to an absolute file URI. The resulting URI will have the format
// "file:///absolute/path/to/file".
inline auto toAbsFilePathUri(const std::filesystem::path& p) -> const Path
{
  Path uri("file://");

  uri += std::filesystem::absolute(p.c_str()).string();

  return uri;
}

// Convert a file URI to a regular file path. If the URI does not start with "file://", an empty
// path is returned.
inline auto fromAbsFilePathUri(const std::string uriPath) -> const Path
{
  Path path;

  if (uriPath.starts_with("file://"))
  {
    path += uriPath.substr(7);
  }

  return path;
}

} // namespace utils::fs
