#pragma once

#include "IDs.hpp"
#include "Store.hpp"

namespace telemetry
{

struct Context
{
  explicit Context(double minPlaybackTimeSecs) : store(minPlaybackTimeSecs) {}

  Store    store;
  Registry registry;

  bool isStoreLoaded    = false;
  bool isRegistryLoaded = false;
};

} // namespace telemetry
