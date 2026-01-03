#pragma once

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
  static void loadFrom(const std::string& path);

  static auto isLoaded() noexcept -> bool;

  // ------------------------------------------------------------
  // Optional accessors (fallback-based)
  // ------------------------------------------------------------
  static auto getString(std::string_view section, std::string_view key,
                        std::string_view fallback = "") -> std::string_view;

  static auto getInt(std::string_view section, std::string_view key, int64_t fallback = -1)
    -> int64_t;

  static auto getBool(std::string_view section, std::string_view key, bool fallback = false)
    -> bool;

  // ------------------------------------------------------------
  // Required accessors (validation)
  // ------------------------------------------------------------
  static auto requireString(std::string_view section, std::string_view key) -> std::string;

  static auto requireInt(std::string_view section, std::string_view key) -> int64_t;

  static auto requireBool(std::string_view section, std::string_view key) -> bool;

  // ------------------------------------------------------------
  // Table access
  // ------------------------------------------------------------
  static auto table(std::string_view section) -> const toml::table&;

private:
  static std::optional<toml::parse_result> s_config;

  static void ensureLoaded();
  static void throwMissing(std::string_view section, std::string_view key);
};

} // namespace tomlparser
