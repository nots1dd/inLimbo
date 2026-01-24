#pragma once

#include "InLimbo-Types.hpp"
#include <unordered_map>

namespace audio::service
{

// we store mapping of SoundHandle ID to Song file path stored in metadata (verified metadata)
//
// This is unique to each song (almost as unique as the inode itself) and more importantly,
// makes it easy to immediately load the song file without calling song map queries.
using TrackTable = std::unordered_map<ui64, const Song*>;

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
