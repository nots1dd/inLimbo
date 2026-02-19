#include "audio/Registry.hpp"

namespace audio
{

auto enumerateBackends() -> BackendList
{
  BackendList backends;

#if defined(__linux__)
  backends.push_back({
    .name        = "ALSA",
    .description = "Advanced Linux Sound Architecture",
    .available   = true,
  });
#endif

#if defined(HAVE_PIPEWIRE)
  backends.push_back({
    .name        = "PipeWire",
    .description = "PipeWire low-latency audio server",
    .available   = true,
  });
#endif

  return backends;
}

} // namespace audio
