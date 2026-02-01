#pragma once

#include "InLimbo-Types.hpp"
#include "utils/ASCII.hpp"
#include "utils/string/SmallString.hpp"
#include "utils/string/Transforms.hpp"
#include <optional>

using KeyName = utils::string::SmallString;

namespace config::keybinds
{

using KeyChar = char;

struct Keybind
{
  KeyChar key{0};

  constexpr Keybind() = default;
  constexpr explicit Keybind(KeyChar k) : key(k) {}
};

constexpr auto key(KeyChar c) -> Keybind { return Keybind{c}; }

enum class Policy : ui8
{
  Adaptive, // respect hardcoded defaults / host behavior
  Forced    // use config-defined keys absolutely
};

static inline auto parseKeyName(KeyChar c) -> KeyName
{
  switch ((unsigned char)c)
  {
    case utils::ascii::NUL:
      return "<none>";
    case utils::ascii::SPACE:
      return "Space";
    case utils::ascii::ENTER:
      return "Enter";
    case utils::ascii::TAB:
      return "Tab";
    case utils::ascii::ESC:
      return "Escape";
    case utils::ascii::DEL:
      return "Backspace";
    default:
      break;
  }

  if (utils::ascii::isPrintable(c))
  {
    return {c};
  }

  return "<unknown>";
}

static inline auto parseSingleChar(std::string_view s) -> std::optional<KeyChar>
{
  if (s.empty())
    return std::nullopt;

  utils::string::SmallString lowered = utils::string::transform::tolower_ascii(s.data());

  if (lowered.size() == 1)
    return s[0];

  if (lowered == "space")
    return utils::ascii::SPACE;

  if (lowered == "enter")
    return utils::ascii::ENTER;

  if (lowered == "tab")
    return utils::ascii::TAB;

  if (lowered == "escape")
    return utils::ascii::ESC;

  return std::nullopt;
}

} // namespace config::keybinds
