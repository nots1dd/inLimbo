#pragma once

#include "IDs.hpp"
#include "Store.hpp"

namespace telemetry
{

struct Context
{
  Store    store;
  Registry registry;

  bool isStoreLoaded    = false;
  bool isRegistryLoaded = false;
};

} // namespace telemetry
