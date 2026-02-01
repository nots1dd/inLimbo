#pragma once

#include "Sound.hpp"
#include "utils/string/SmallString.hpp"
#include <algorithm>
#include <optional>
#include <span>
#include <thread>

extern "C"
{
#include <alsa/asoundlib.h>
}

namespace audio
{

using DeviceName = utils::string::SmallString;
using CodecName  = utils::string::SmallString;

struct Device
{
  DeviceName                 name;
  utils::string::SmallString description;
  int                        cardIndex   = -1;
  int                        deviceIndex = -1;
  bool                       isDefault   = false;
};

struct BackendInfo
{
  Device dev;

  // ---------------------------------------------------------
  // Audio format (NEGOTIATED, not requested)
  // ---------------------------------------------------------
  uint                       sampleRate = DEFAULT_SOUND_SAMPLE_RATE;
  uint                       channels   = DEFAULT_SOUND_CHANNELS;
  snd_pcm_format_t           pcmFormat  = SND_PCM_FORMAT_UNKNOWN;
  utils::string::SmallString pcmFormatName;

  CodecName codecName;     // "flac", "mp3", "aac", ...
  CodecName codecLongName; // "FLAC (Free Lossless Audio Codec)", etc.

  snd_pcm_uframes_t periodSize = 0; // frames
  snd_pcm_uframes_t bufferSize = 0; // frames
  double            latencyMs  = 0.0;

  bool isActive   = false;
  bool isPlaying  = false;
  bool isPaused   = false;
  bool isDraining = false;

  ui64 xruns  = 0; // underrun count
  ui64 writes = 0; // snd_pcm_writei calls
};

using Devices = std::vector<Device>;

enum class PlaybackState : ui8
{
  Stopped,
  Playing,
  Paused
};

class AudioEngine final
{
public:
  AudioEngine() { av_log_set_level(AV_LOG_ERROR); }

  ~AudioEngine() noexcept
  {
    stop();
    cleanup();
    shutdownAlsa();
  }

  // Properly enumerate ALSA devices
  auto enumeratePlaybackDevices() -> Devices;
  void initEngineForDevice(const DeviceName& deviceName = "default");

  auto loadSound(const Path& path) -> bool;
  auto queueNextSound(const Path& path) -> bool;

  void play();
  void pause();
  void stop();

  void restart()
  {
    seekToAbsolute(0.0);
    play();
  }

  [[nodiscard]] auto isPlaying() const -> bool { return m_playbackState == PlaybackState::Playing; }

  auto getPlaybackTime() const -> std::optional<std::pair<double, double>>;

  void seekToAbsolute(double seconds);
  void seekForward(double seconds);
  void seekBackward(double seconds);

  void setVolume(float v) { m_volume.store(std::clamp(v, 0.0f, 1.5f)); }

  auto getBackendInfo() const -> BackendInfo
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_backendInfo;
  }

  [[nodiscard]] auto getVolume() const -> float { return m_volume.load(); }

  [[nodiscard]] auto getCurrentDevice() const -> DeviceName
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentDevice;
  }

  auto getSoundPtrMut() const -> std::shared_ptr<Sound>;
  auto getSoundPtr() const -> std::shared_ptr<const Sound>;

  template <typename Fn> auto returnAudioBuffersView(Fn&& fn) const -> void;
  auto                        getCopySeq() const noexcept -> ui64;
  auto                        getCopyBufferSize() const noexcept -> size_t;

private:
  snd_pcm_t*  m_pcmData = nullptr;
  Floats      m_playbackBuffer;
  Bytes       m_scratchBuffer;
  BackendInfo m_backendInfo;
  DeviceName  m_currentDevice = "default";

  std::shared_ptr<audio::Sound> m_sound;
  std::shared_ptr<audio::Sound> m_nextSound;
  std::atomic<float>            m_volume{1.0f};
  std::atomic<bool>             m_isRunning{false};
  std::atomic<PlaybackState>    m_playbackState{PlaybackState::Stopped};
  std::thread                   m_audioThread;
  mutable std::mutex            m_mutex;

  // this is to visit and copy audio buffers and returnAudioBuffersView().
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
    m_audioThread = std::thread(&AudioEngine::audioLoop, this);
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

template <typename Fn> auto AudioEngine::returnAudioBuffersView(Fn&& fn) const -> void
{
  std::lock_guard<std::mutex> copyLock(m_copyMutex);

  if (m_copyBuffer.empty())
  {
    fn(std::span<const float>{});
    return;
  }

  fn(std::span<const float>{m_copyBuffer.data(), m_copyBuffer.size()});
}

} // namespace audio
