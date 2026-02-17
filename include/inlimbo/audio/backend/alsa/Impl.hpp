#pragma once

#include "audio/backend/Backend.hpp"
#include "audio/backend/Interface.hpp"
#include <algorithm>
#include <thread>

extern "C"
{
#include <alsa/asoundlib.h>
}

namespace audio::backend
{

class AlsaBackend : public IAudioBackend
{
public:
  static constexpr std::string_view kID        = "alsa";
  static constexpr std::string_view kName      = "AlsaBackend";
  static constexpr BackendID        kBackendID = BackendID::Alsa;

  auto backendID() const noexcept -> BackendID override { return kBackendID; }
  auto backendString() const noexcept -> std::string_view override { return kID; }

  auto enumerateDevices() -> Devices override;
  void initForDevice(const DeviceName& deviceName = "default") override;
  void switchDevice(const DeviceName& device) override;

  [[nodiscard]] auto currentDevice() const -> DeviceName override
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentDevice;
  }

  auto load(const Path& path) -> bool override;
  auto queueNext(const Path& path) -> bool override;

  void play() override;
  void pause() override;
  void stop() override;

  void restart() override
  {
    seekAbsolute(0.0);
    play();
  }

  [[nodiscard]] auto isPlaying() const -> bool { return m_playbackState == PlaybackState::Playing; }

  auto state() const -> PlaybackState override { return m_playbackState.load(); }

  auto playbackTime() const -> std::optional<std::pair<double, double>> override;

  void seekAbsolute(double seconds) override;
  void seekForward(double seconds) override;
  void seekBackward(double seconds) override;

  void setVolume(float v) override { m_volume.store(std::clamp(v, 0.0f, 1.5f)); }

  auto getBackendInfo() const -> BackendInfo override
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_backendInfo;
  }

  [[nodiscard]] auto volume() const -> float override { return m_volume.load(); }

  auto isTrackFinished() const -> bool override { return m_trackFinished.load(); }
  auto clearTrackFinished() -> void override
  {
    m_trackFinished.store(false, std::memory_order_release);
  }

  auto getSoundPtrMut() const -> std::shared_ptr<Sound>;
  auto getSoundPtr() const -> std::shared_ptr<const Sound>;

  auto copySequence() const noexcept -> ui64 override;
  auto copyBufferSize() const noexcept -> size_t override;

  template <typename Fn>
  void withAudioBuffer(Fn&& fn) const
  {
    visitAudioBuffer(std::forward<Fn>(fn));
  }

protected:
  void visitAudioBuffer(void (*fn)(const float*, size_t)) const override
  {
    std::lock_guard<std::mutex> lock(m_copyMutex);
    if (!m_copyBuffer.empty())
      fn(m_copyBuffer.data(), m_copyBuffer.size());
  }

private:
  snd_pcm_t*        m_pcmData = nullptr;
  Floats            m_playbackBuffer;
  Bytes             m_scratchBuffer;
  BackendInfo       m_backendInfo;
  DeviceName        m_currentDevice = "default";
  std::atomic<bool> m_switchPending{false};
  DeviceName        m_pendingDevice;

  std::shared_ptr<audio::Sound> m_sound;
  std::shared_ptr<audio::Sound> m_nextSound;
  std::atomic<float>            m_volume{1.0f};
  std::atomic<bool>             m_isRunning{false};
  std::atomic<bool>             m_trackFinished{false};
  std::atomic<PlaybackState>    m_playbackState{PlaybackState::Stopped};
  std::thread                   m_audioThread;
  mutable std::mutex            m_mutex;

  // this is to visit and copy audio buffers and withAudioBuffer().
  // useful for audio visualization
  mutable std::mutex m_copyMutex;
  Floats             m_copyBuffer;
  std::atomic<ui64>  m_copySeq{0};
  size_t             m_copySamples = 0;

  void startThread()
  {
    if (m_isRunning)
      return;
    m_isRunning   = true;
    m_audioThread = std::thread(&AlsaBackend::audioLoop, this);
  }

  void initAlsa(const DeviceName& deviceName = "default");
  void shutdownAlsa();

  void audioLoop()
  {
    while (m_isRunning)
    {
      if (m_playbackState == PlaybackState::Playing)
        decodeAndPlay();
      else
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  void switchAlsaDevice();

  auto prepareSound(const Path& path) -> std::shared_ptr<Sound>;
  auto returnNextSoundIfQueued() -> std::shared_ptr<Sound>;
  void decodeAndPlay();
  void decodeStep(Sound& s);

  void cleanup()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sound.reset();
  }
};

} // namespace audio::backend
