#pragma once

#include "InLimbo-Types.hpp"
#include <string>

namespace mpris
{

enum class LoopMode
{
  None,
  Track,
  Playlist
};

class IMprisBackend
{
public:
  virtual ~IMprisBackend() = default;

  /* Playback control */
  virtual void play()     = 0;
  virtual void pause()    = 0;
  virtual void stop()     = 0;
  virtual void next()     = 0;
  virtual void previous() = 0;

  /* Seeking */
  virtual void seekSeconds(double offset)     = 0; // relative
  virtual void setPositionSeconds(double pos) = 0; // absolute

  /* State */
  [[nodiscard]] virtual auto isPlaying() const -> bool         = 0;
  [[nodiscard]] virtual auto positionSeconds() const -> double = 0;
  [[nodiscard]] virtual auto durationSeconds() const -> double = 0;

  /* Metadata */
  [[nodiscard]] virtual auto title() const -> Title  = 0;
  [[nodiscard]] virtual auto artist() const -> Artist = 0;
  [[nodiscard]] virtual auto album() const -> Album  = 0;
  [[nodiscard]] virtual auto artUrl() const -> std::string = 0;

  /* Volume */
  [[nodiscard]] virtual auto volume() const -> double = 0; // 0.0 → 1.0
  virtual void               setVolume(double v)      = 0;
};

} // namespace mpris
