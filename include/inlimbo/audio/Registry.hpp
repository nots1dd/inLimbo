#pragma once

#include <vector>

namespace audio
{

struct BackendDescriptor
{
  const char* name;        // "ALSA", "PipeWire", "WASAPI"
  const char* description; // Human-readable
  bool        available;   // Compiled + usable on this system
};

using BackendList = std::vector<BackendDescriptor>;

auto enumerateBackends() -> BackendList;

} // namespace audio
