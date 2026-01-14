#pragma once

#include "InLimbo-Types.hpp"
#include "toml/Types.hpp"
#include <optional>
#include <string>
#include <string_view>

#include <toml.hpp>

namespace tomlparser
{

class Config
{
public:
  static void load();
  static void loadFrom(const Path& path);

  static auto isLoaded() noexcept -> bool;

  // ------------------------------------------------------------
  // Optional accessors (fallback-based)
  // ------------------------------------------------------------
  static auto getString(SectionView section, KeyView key, std::string_view fallback = "")
    -> std::string_view;

  static auto getInt(SectionView section, KeyView key, i64 fallback = -1) -> int64_t;

  static auto getBool(SectionView section, KeyView key, bool fallback = false) -> bool;

  // ------------------------------------------------------------
  // Required accessors (validation)
  // ------------------------------------------------------------
  static auto requireString(SectionView section, KeyView key) -> std::string;

  static auto requireInt(SectionView section, KeyView key) -> i64;

  static auto requireBool(SectionView section, KeyView key) -> bool;

  // ------------------------------------------------------------
  // Table access
  // ------------------------------------------------------------
  static auto table(SectionView section) -> const toml::table&;

private:
  static std::optional<toml::parse_result> s_config;

  static void ensureLoaded();
  static void throwMissing(SectionView section, KeyView key);
};

} // namespace tomlparser
