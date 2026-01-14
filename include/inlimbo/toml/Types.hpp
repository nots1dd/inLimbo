#pragma once

#include <string_view>

namespace tomlparser
{

template <typename T> using Key = T;

template <typename T> using Value = T;

using KeyView   = std::string_view;
using ValueView = std::string_view;

using SectionView = std::string_view;

} // namespace tomlparser
