#pragma once

#include "audio/backend/Devices.hpp"
#include <memory>
#include <optional>
#include <utility>

namespace audio
{

enum class PlaybackState : ui8
{
  Stopped,
  Playing,
  Paused
};

class IAudioBackend
{
public:
  virtual ~IAudioBackend() = default;

  /* ---------------- Device management ---------------- */

  virtual auto               enumerateDevices() -> Devices           = 0;
  virtual void               initForDevice(const DeviceName& device) = 0;
  virtual void               switchDevice(const DeviceName& device)  = 0;
  [[nodiscard]] virtual auto currentDevice() const -> DeviceName     = 0;

  /* ---------------- Playback control ---------------- */

  virtual auto load(const Path& path) -> bool      = 0;
  virtual auto queueNext(const Path& path) -> bool = 0;

  virtual void play()    = 0;
  virtual void pause()   = 0;
  virtual void stop()    = 0;
  virtual void restart() = 0;

  /* ---------------- Playback state ---------------- */

  [[nodiscard]] virtual auto state() const -> PlaybackState  = 0;
  [[nodiscard]] virtual auto isTrackFinished() const -> bool = 0;
  virtual void               clearTrackFinished()            = 0;

  [[nodiscard]] virtual auto playbackTime() const -> std::optional<std::pair<double, double>> = 0;

  virtual void seekAbsolute(double seconds) = 0;
  virtual void seekForward(double seconds)  = 0;
  virtual void seekBackward(double seconds) = 0;

  /* ---------------- Volume ---------------- */

  virtual void               setVolume(float volume) = 0;
  [[nodiscard]] virtual auto volume() const -> float = 0;

  [[nodiscard]] virtual auto getBackendInfo() const -> BackendInfo = 0;

  /* ---------------- Visualization support ---------------- */

  [[nodiscard]] virtual auto copySequence() const noexcept -> ui64     = 0;
  [[nodiscard]] virtual auto copyBufferSize() const noexcept -> size_t = 0;

  template <typename Fn>
  void withAudioBuffer(Fn&& fn) const
  {
    visitAudioBuffer(std::forward<Fn>(fn));
  }

protected:
  virtual void visitAudioBuffer(void (*fn)(const float*, size_t)) const = 0;
};

using AudioBackendPtr = std::unique_ptr<IAudioBackend>;

} // namespace audio
