#pragma once

#include "ankerl/unordered_dense.h"
#include "utils/ankerl/Cereal.hpp"
#include "utils/map/AllocOptimization.hpp"
#include <cereal/types/string.hpp>

namespace utils::string
{

template <typename ID>
class StringTable
{
public:
  auto getOrCreate(std::string_view s) -> ID
  {
    if (auto it = strToId.find(s); it != strToId.end())
      return it->second;

    ID id = nextId++;

    strToId.emplace(std::string(s), id);

    if (id >= idToStr.size())
      idToStr.resize(id + 1);

    idToStr[id] = std::string(s);

    return id;
  }

  auto toString(ID id) const -> std::optional<std::string_view>
  {
    if (id == 0 || id >= idToStr.size())
      return std::nullopt;

    return std::string_view{idToStr[id]};
  }

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(strToId, idToStr, nextId);
  }

private:
  ID nextId{1};

  ankerl::unordered_dense::map<std::string, ID, map::TransparentHash, map::TransparentEq> strToId;
  std::vector<std::string>                                                                idToStr;
};

} // namespace utils::string
