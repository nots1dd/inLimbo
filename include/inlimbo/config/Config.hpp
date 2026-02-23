#pragma once

#include "InLimbo-Types.hpp"
#include "config/Types.hpp"
#include <optional>
#include <string>
#include <string_view>

#include <toml.hpp>

namespace config
{

// this class is named as Config due to
// it's static methods that will be
// called to fetch values from the
// config file.
//
// In reality it is a pretty generic toml
// value fetching class but this class
// must be used explicitly for fetching
// and loading inLimbo's config.toml only.
//
// (seemed reasonable to me, open to changing)

class Config
{
public:
  static void load();
  static void loadFrom(const Path& path);

  static auto isLoaded() noexcept -> bool;

  // ------------------------------------------------------------
  // Optional accessors (fallback-based)
  // ------------------------------------------------------------
  static auto getString(toml::SectionView section, toml::KeyView key,
                        std::string_view fallback = "") -> std::string_view;

  static auto getInt(toml::SectionView section, toml::KeyView key, i64 fallback = -1) -> i64;

  static auto getBool(toml::SectionView section, toml::KeyView key, bool fallback = false) -> bool;

  // ------------------------------------------------------------
  // Required accessors (validation)
  // ------------------------------------------------------------
  static auto requireString(toml::SectionView section, toml::KeyView key) -> std::string;

  static auto requireInt(toml::SectionView section, toml::KeyView key) -> i64;

  static auto requireBool(toml::SectionView section, toml::KeyView key) -> bool;

  // ------------------------------------------------------------
  // Table access
  // ------------------------------------------------------------
  static auto table(toml::SectionView section) -> const ::toml::table&;

private:
  static std::optional<::toml::parse_result> s_config;

  static void ensureLoaded();
  static void throwMissing(toml::SectionView section, toml::KeyView key);
};

} // namespace config
