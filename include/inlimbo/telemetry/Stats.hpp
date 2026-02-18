#pragma once

#include "IDs.hpp"

namespace telemetry
{

struct Stats
{
  uint32_t playCount = 0;
  double   listenSec = 0.0;

  Timestamp first = 0;
  Timestamp last  = 0;

  inline void addListen(double sec) { listenSec += sec; }

  inline void commitPlay(Timestamp ts)
  {
    ++playCount;

    if (first == 0)
      first = ts;
    last = ts;
  }

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(playCount, listenSec, first, last);
  }
};

} // namespace telemetry
