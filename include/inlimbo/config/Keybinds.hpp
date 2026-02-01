#pragma once

#include "KBUtils.hpp"

#include "utils/map/AllocOptimization.hpp"
#include "utils/string/SmallString.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

template <typename T>
concept KeyValueType = std::is_same_v<T, std::string> || std::is_same_v<T, char> ||
                       std::is_same_v<T, int> || std::is_same_v<T, float>;

namespace config::keybinds
{

using Member = utils::string::SmallString;

class Theme
{
public:
  Theme() = default;
  explicit Theme(std::string name) : m_name(std::move(name)) {}

  [[nodiscard]] auto name() const noexcept -> const std::string& { return m_name; }

  auto set(std::string action, Keybind bind) -> Theme&
  {
    m_binds[std::move(action)] = bind;
    return *this;
  }

  [[nodiscard]] auto has(std::string_view action) const -> bool
  {
    return m_binds.find(action) != m_binds.end();
  }

  [[nodiscard]] auto get(std::string_view action, Keybind fallback = {}) const -> Keybind
  {
    auto it = m_binds.find(action);
    if (it == m_binds.end())
      return fallback;
    return it->second;
  }

  // Generic typed getter
  //
  // Examples:
  //   int  k = theme.getAs<int>("play_pause", KEY_SPACE);      // for raylib (example)
  //   auto e = theme.getAs<float>("vol", 100.0);               // explicit dtype conv
  //   auto v = theme.getAs<char>("quit", 'q');                 // general use case
  //
  template <KeyValueType T> auto getAs(std::string_view action, T fallback = {}) const -> T
  {
    auto it = m_binds.find(action);
    if (it == m_binds.end())
      return fallback;

    return static_cast<T>(it->second.key);
  }

  [[nodiscard]] auto getKeyName(std::string_view action, std::string_view fallback = "<none>") const
    -> utils::string::SmallString
  {
    auto it = m_binds.find(action);
    if (it == m_binds.end())
      return {fallback};

    return parseKeyName(it->second.key);
  }

private:
  std::string m_name{"default"};

  std::unordered_map<std::string, Keybind, utils::map::TransparentHash, utils::map::TransparentEq>
    m_binds;
};

class Registry
{
public:
  static auto setActive(std::string name) -> void { s_active = std::move(name); }
  static auto activeName() -> const std::string& { return s_active; }

  static auto put(Theme theme) -> void { s_themes[theme.name()] = std::move(theme); }

  static auto theme(std::string_view name) -> Theme& { return s_themes[std::string(name)]; }

  static auto active() -> Theme& { return s_themes[s_active]; }

  static auto setForced(bool enabled) -> void { s_forceKeybinds = enabled; }
  static auto forced() noexcept -> bool { return s_forceKeybinds; }

private:
  static inline std::unordered_map<std::string, Theme> s_themes{};
  static inline std::string                            s_active{"default"};
  static inline bool                                   s_forceKeybinds{true};
};

} // namespace config::keybinds
