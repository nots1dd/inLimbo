#pragma once

#include "InLimbo-Types.hpp"
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
    case 0:
      return "<none>";
    case ' ':
      return "Space";
    case '\n':
      return "Enter";
    case '\t':
      return "Tab";
    case 27:
      return "Escape";
    case 127:
      return "Backspace";
    default:
      break;
  }

  if ((unsigned char)c >= 32 && (unsigned char)c <= 126)
  {
    utils::string::SmallString s;
    s += c;
    return s;
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
    return ' ';

  if (lowered == "enter")
    return '\n';

  if (lowered == "tab")
    return '\t';

  if (lowered == "escape")
    return 27;

  return std::nullopt;
}

} // namespace config::keybinds
