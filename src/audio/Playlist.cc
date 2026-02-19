#include "audio/Playlist.hpp"
#include "utils/Index.hpp"
#include "utils/RNG.hpp"

namespace audio::service
{

auto Playlist::empty() const -> bool { return tracks.empty(); }

auto Playlist::size() const noexcept -> size_t { return tracks.size(); }

auto Playlist::currentTrack() const -> std::optional<Handle>
{
  if (tracks.empty())
    return std::nullopt;

  return tracks[current];
}

auto Playlist::next() noexcept -> std::optional<Handle>
{
  if (tracks.empty())
    return std::nullopt;

  current = *utils::index::nextWrap(current, tracks.size());

  return tracks[current];
}

auto Playlist::previous() noexcept -> std::optional<Handle>
{
  if (tracks.empty())
    return std::nullopt;

  current = *utils::index::prevWrap(current, tracks.size());

  return tracks[current];
}

// playlist refs work with references to avoid copying handles
//
// Will return the const reference to the handle in the playlist
auto Playlist::currentTrackRef() const noexcept -> std::optional<HandleRef>
{
  if (tracks.empty())
    return std::nullopt;

  return std::cref(tracks[current]);
}

auto Playlist::nextRef() noexcept -> std::optional<HandleRef>
{
  if (tracks.empty())
    return std::nullopt;

  current = *utils::index::nextWrap(current, tracks.size());

  return std::cref(tracks[current]);
}

auto Playlist::previousRef() noexcept -> std::optional<HandleRef>
{
  if (tracks.empty())
    return std::nullopt;

  current = *utils::index::prevWrap(current, tracks.size());

  return std::cref(tracks[current]);
}

auto Playlist::jumpToIndex(size_t idx) noexcept -> std::optional<Handle>
{
  if (tracks.empty())
    return std::nullopt;

  if (idx >= tracks.size())
    return std::nullopt;

  current = idx;
  return tracks[current];
}

// does not allow jumping to the same track
auto Playlist::jumpToRandom() noexcept -> std::optional<Handle>
{
  if (tracks.empty())
    return std::nullopt;

  if (tracks.size() == 1)
  {
    current = 0;
    return tracks[0];
  }

  size_t idx = current;
  while (idx == current)
    idx = (size_t)utils::RNG::u32(0u, (ui32)tracks.size() - 1);

  current = idx;
  return tracks[current];
}

auto Playlist::randomIndex() const noexcept -> std::optional<size_t>
{
  if (tracks.empty())
    return std::nullopt;

  return static_cast<size_t>(utils::RNG::u32(0u, static_cast<ui32>(tracks.size() - 1)));
}

void Playlist::clear()
{
  tracks.clear();
  current = 0;
}

void Playlist::removeAt(size_t index) noexcept
{
  if (index >= tracks.size())
    return;

  tracks.erase(tracks.begin() + index);

  if (tracks.empty())
  {
    current = 0;
    return;
  }

  // Adjust current idx
  if (current > index)
  {
    --current;
  }
  else if (current >= tracks.size())
  {
    current = tracks.size() - 1;
  }
}

} // namespace audio::service
