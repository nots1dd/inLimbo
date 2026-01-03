#pragma once

#include "InLimbo-Types.hpp"
#include "core/taglib/Parser.hpp"
#include <unordered_map>

namespace audio::service
{

using TrackTable = std::unordered_map<ui64, std::string>;
using MetadataTable = std::unordered_map<ui64, Metadata>;

struct SoundHandle
{
  ui64     id = 0;
  explicit operator bool() const { return id != 0; }
};

struct TrackInfo
{
  double positionSec = 0.0;
  double lengthSec   = 0.0;

  ui32 sampleRate = 0;
  ui32 channels   = 0;

  std::string format;
  bool        playing = false;
};

} // namespace audio::service
