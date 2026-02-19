#pragma once

#include <optional>
#include <string_view>
#include <sys/stat.h>

namespace utils::fs
{

inline auto inodeFromPath(std::string_view path) noexcept -> std::optional<ino_t>
{
  struct stat st{};
  if (::stat(path.data(), &st) != 0)
    return std::nullopt;

  return st.st_ino;
}

} // namespace utils::fs
