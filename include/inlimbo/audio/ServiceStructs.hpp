#pragma once

#include "InLimbo-Types.hpp"

namespace audio::service
{

// we store mapping of SoundHandle ID to a shared pointer to the song object
//
// makes it easy to immediately load the song file without calling song map queries.
using TrackTable = ankerl::unordered_dense::map<ui64, std::shared_ptr<const Song>>;

struct SoundHandle
{
  ui64     id = 0;
  explicit operator bool() const { return id != 0; }
};

// can be identified by a 8b unique counter
struct TrackInfo
{
  ui8    tid         = 0;
  double positionSec = 0.0;
  double lengthSec   = 0.0;

  ui32 sampleRate = 0;
  ui32 channels   = 0;

  std::string format;
  bool        playing = false;
};

} // namespace audio::service
