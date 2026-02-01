#pragma once

#include "InLimbo-Types.hpp"
#include <array>
#include <optional>
#include <string_view>

namespace utils::colors
{

struct RGBA
{
  ui8 r{0};
  ui8 g{0};
  ui8 b{0};
  ui8 a{255};

  constexpr RGBA() = default;
  constexpr RGBA(ui8 rr, ui8 gg, ui8 bb, ui8 aa = 255) : r(rr), g(gg), b(bb), a(aa) {}
};

constexpr auto rgba(ui8 r, ui8 g, ui8 b, ui8 a = 255) -> RGBA { return RGBA{r, g, b, a}; }

// "#RRGGBB" or "#RRGGBBAA"
constexpr auto hexNibble(char c) -> int
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  return -1;
}

constexpr auto hexByte(char a, char b) -> int
{
  const int hi = hexNibble(a);
  const int lo = hexNibble(b);
  if (hi < 0 || lo < 0)
    return -1;
  return (hi << 4) | lo;
}

constexpr auto fromHex(std::string_view hex, RGBA fallback = RGBA{}) -> RGBA
{
  if (hex.empty())
    return fallback;

  size_t i = 0;
  if (hex[0] == '#')
    i = 1;

  const size_t n = hex.size() - i;
  if (!(n == 6 || n == 8))
    return fallback;

  const int r = hexByte(hex[i + 0], hex[i + 1]);
  const int g = hexByte(hex[i + 2], hex[i + 3]);
  const int b = hexByte(hex[i + 4], hex[i + 5]);
  if (r < 0 || g < 0 || b < 0)
    return fallback;

  int a = 255;
  if (n == 8)
  {
    const int aa = hexByte(hex[i + 6], hex[i + 7]);
    a            = (aa < 0) ? 255 : aa;
  }

  return RGBA{(ui8)r, (ui8)g, (ui8)b, (ui8)a};
}

inline auto parseRGBA(std::string_view s) -> std::optional<RGBA>
{
  // ---------- #RRGGBB or #RRGGBBAA ----------
  if (s.size() == 7 || s.size() == 9)
  {
    if (s[0] != '#')
      return std::nullopt;

    int r = hexByte(s[1], s[2]);
    int g = hexByte(s[3], s[4]);
    int b = hexByte(s[5], s[6]);
    if (r < 0 || g < 0 || b < 0)
      return std::nullopt;

    int a = 255;
    if (s.size() == 9)
    {
      a = hexByte(s[7], s[8]);
      if (a < 0)
        return std::nullopt;
    }

    return RGBA{(ui8)r, (ui8)g, (ui8)b, (ui8)a};
  }

  // ---------- r,g,b[,a] ----------
  std::array<int, 4> vals      = {0, 0, 0, 255};
  int                idx       = 0;
  int                cur       = 0;
  bool               haveDigit = false;

  for (char c : s)
  {
    if (c >= '0' && c <= '9')
    {
      haveDigit = true;
      cur       = cur * 10 + (c - '0');
      if (cur > 255)
        return std::nullopt;
    }
    else if (c == ',' && haveDigit)
    {
      if (idx >= 4)
        return std::nullopt;
      vals[idx++] = cur;
      cur         = 0;
      haveDigit   = false;
    }
    else
    {
      return std::nullopt;
    }
  }

  if (!haveDigit || idx < 2 || idx > 3)
    return std::nullopt;

  vals[idx] = cur;

  return RGBA{(ui8)vals[0], (ui8)vals[1], (ui8)vals[2], (ui8)vals[3]};
}

} // namespace utils::colors
