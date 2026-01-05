#pragma once

#include "utils/string/SmallString.hpp"
#include <fmt/core.h>

template <> struct fmt::formatter<utils::string::SmallString>
{
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const utils::string::SmallString& s, FormatContext& ctx) const
  {
    return fmt::format_to(ctx.out(), "{}", s.c_str());
  }
};
