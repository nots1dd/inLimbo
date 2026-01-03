#pragma once

#include "ServiceStructs.hpp"

namespace audio::service
{

struct Playlist
{
  std::vector<service::SoundHandle> tracks;
  size_t                            current = 0;

  [[nodiscard]] auto empty() const -> bool { return tracks.empty(); }

  [[nodiscard]] auto currentTrack() const -> service::SoundHandle
  {
    if (tracks.empty())
      return {};
    return tracks[current];
  }

  auto next() -> service::SoundHandle
  {
    if (tracks.empty())
      return {};
    current = (current + 1) % tracks.size();
    return tracks[current];
  }

  auto previous() -> service::SoundHandle
  {
    if (tracks.empty())
      return {};
    current = (current + tracks.size() - 1) % tracks.size();
    return tracks[current];
  }

  void clear()
  {
    tracks.clear();
    current = 0;
  }
};

} // namespace audio::service
