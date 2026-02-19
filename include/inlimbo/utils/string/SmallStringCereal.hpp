#pragma once

#include <cereal/cereal.hpp>
#include <string>

#include "utils/string/SmallString.hpp"

namespace utils::string
{

template <class Archive>
void save(Archive& ar, const SmallString& s)
{
  // Serialize as a normal string
  std::string tmp{s.c_str(), s.size()};
  ar(tmp);
}

template <class Archive>
void load(Archive& ar, SmallString& s)
{
  std::string tmp;
  ar(tmp);

  s.clear();
  s += tmp.c_str();
}

} // namespace utils::string
