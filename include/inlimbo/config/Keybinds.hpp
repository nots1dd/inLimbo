#pragma once

#include "KBUtils.hpp"

namespace config::keybinds
{

using Member = utils::string::SmallString;

struct KeybindValue
{
  Keybind bind{};

  constexpr KeybindValue() = default;
  constexpr explicit KeybindValue(Keybind b) : bind(b) {}
};

class Theme
{
public:
  Theme() = default;
  explicit Theme(std::string name) : m_name(std::move(name)) {}

  auto name() const noexcept -> const std::string& { return m_name; }

  auto set(std::string action, Keybind bind) -> Theme&
  {
    m_binds[std::move(action)] = KeybindValue{bind};
    return *this;
  }

  auto has(std::string_view action) const -> bool
  {
    return m_binds.find(std::string(action)) != m_binds.end();
  }

  auto get(std::string_view action, Keybind fallback = {}) const -> Keybind
  {
    auto it = m_binds.find(std::string(action));
    if (it == m_binds.end())
      return fallback;
    return it->second.bind;
  }

  auto getChar(std::string_view action, KeyChar fallback = 0) const -> KeyChar
  {
    auto it = m_binds.find(std::string(action));
    if (it == m_binds.end())
      return fallback;
    return it->second.bind.key;
  }

  auto getKeyName(std::string_view action, std::string_view fallback = "<none>") const
    -> utils::string::SmallString
  {
    auto it = m_binds.find(std::string(action));
    if (it == m_binds.end())
      return {fallback};

    return parseKeyName(it->second.bind.key);
  }

private:
  std::string                                   m_name{"default"};
  std::unordered_map<std::string, KeybindValue> m_binds;
};

class Registry
{
public:
  static auto setActive(std::string name) -> void { s_active = std::move(name); }
  static auto activeName() -> const std::string& { return s_active; }

  static auto put(Theme theme) -> void { s_themes[theme.name()] = std::move(theme); }

  static auto theme(std::string_view name) -> Theme& { return s_themes[std::string(name)]; }

  static auto active() -> Theme& { return s_themes[s_active]; }

  // global toggle: whether we enforce configured keys or allow defaults
  static auto setForced(bool enabled) -> void { s_forceKeybinds = enabled; }
  static auto forced() noexcept -> bool { return s_forceKeybinds; }

private:
  static inline std::unordered_map<std::string, Theme> s_themes{};
  static inline std::string                            s_active{"default"};
  static inline bool                                   s_forceKeybinds{true};
};

} // namespace config::keybinds
