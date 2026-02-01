#pragma once

#include "utils/Colors.hpp"
#include <functional>
#include <string_view>

namespace config
{

namespace keybinds
{

template <typename T> struct Binding
{
  std::string_view key;    // TOML key name
  T*               target; // pointer to member

  // Optional custom apply logic
  // (ascii_char, target)
  void (*apply)(char, T&) = nullptr;
};

} // namespace keybinds

namespace colors
{

template <typename T> struct Binding
{
  std::string_view key;
  T*               target;

  std::function<void(const utils::colors::RGBA&, T&)> apply;
};

} // namespace colors

} // namespace config
