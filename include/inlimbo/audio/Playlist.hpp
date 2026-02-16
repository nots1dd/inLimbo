#pragma once

#include "ServiceStructs.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

namespace audio::service
{

struct Playlist
{
  using Handle    = service::SoundHandle;
  using HandleRef = std::reference_wrapper<const Handle>;

  std::vector<Handle> tracks;
  size_t              current = 0;

  [[nodiscard]] auto empty() const -> bool;
  [[nodiscard]] auto size() const noexcept -> size_t;

  [[nodiscard]] auto currentTrack() const -> std::optional<Handle>;
  [[nodiscard]] auto next() noexcept -> std::optional<Handle>;
  [[nodiscard]] auto previous() noexcept -> std::optional<Handle>;

  [[nodiscard]] auto currentTrackRef() const noexcept -> std::optional<HandleRef>;
  [[nodiscard]] auto nextRef() noexcept -> std::optional<HandleRef>;
  [[nodiscard]] auto previousRef() noexcept -> std::optional<HandleRef>;

  [[nodiscard]] auto randomIndex() const noexcept -> std::optional<size_t>;
  [[nodiscard]] auto jumpToIndex(size_t idx) noexcept -> std::optional<Handle>;
  [[nodiscard]] auto jumpToRandom() noexcept -> std::optional<Handle>;

  void clear();
  void removeAt(size_t index) noexcept;
};

} // namespace audio::service
