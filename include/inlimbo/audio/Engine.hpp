#pragma once

#include "Config.hpp"
#include "Sound.hpp"
#include "utils/string/SmallString.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

extern "C"
{
#include <alsa/asoundlib.h>
}

namespace audio
{

using DeviceName = utils::string::SmallString;

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
  uint             sampleRate = DEFAULT_SOUND_SAMPLE_RATE;
  uint             channels   = DEFAULT_SOUND_CHANNELS;
  snd_pcm_format_t pcmFormat  = SND_PCM_FORMAT_UNKNOWN;
  std::string      pcmFormatName;

  snd_pcm_uframes_t periodSize = 0; // frames
  snd_pcm_uframes_t bufferSize = 0; // frames
  double            latencyMs  = 0.0;

  bool isActive   = false;
  bool isPlaying  = false;
  bool isPaused   = false;
  bool isDraining = false;

  uint64_t xruns  = 0; // underrun count
  uint64_t writes = 0; // snd_pcm_writei calls
};

using Devices = std::vector<Device>;

enum class PlaybackState : ui8
{
  Stopped,
  Playing,
  Paused
};

class AudioEngine
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
  INLIMBO_API_CPP auto enumeratePlaybackDevices() -> Devices;
  INLIMBO_API_CPP void
  initEngineForDevice(const utils::string::SmallString& deviceName = "default");

  INLIMBO_API_CPP auto loadSound(const Path& path) -> bool;

  INLIMBO_API_CPP void play();
  INLIMBO_API_CPP void pause();
  INLIMBO_API_CPP void stop();

  INLIMBO_API_CPP void restart()
  {
    seekTo(0.0);
    play();
  }

  INLIMBO_API_CPP [[nodiscard]] auto isPlaying() const -> bool
  {
    return m_playbackState == PlaybackState::Playing;
  }

  INLIMBO_API_CPP auto getPlaybackTime() const -> std::optional<std::pair<double, double>>;

  INLIMBO_API_CPP void seekAbsolute(double seconds);
  INLIMBO_API_CPP void seekTo(double seconds);
  INLIMBO_API_CPP void seekForward(double seconds);
  INLIMBO_API_CPP void seekBackward(double seconds);

  INLIMBO_API_CPP void setVolume(float v) { m_volume.store(std::clamp(v, 0.0f, 1.5f)); }

  INLIMBO_API_CPP auto getBackendInfo() const -> BackendInfo
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_backendInfo;
  }

  INLIMBO_API_CPP [[nodiscard]] auto getVolume() const -> float { return m_volume.load(); }

  INLIMBO_API_CPP [[nodiscard]] auto getCurrentDevice() const -> DeviceName
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentDevice;
  }

  auto getSound() -> Sound&;
  auto getSound() const -> const Sound&;

private:
  snd_pcm_t*  m_pcmData = nullptr;
  BackendInfo m_backendInfo;
  DeviceName  m_currentDevice = "default";

  std::unique_ptr<audio::Sound> m_sound;
  std::atomic<float>            m_volume{1.0f};
  std::atomic<bool>             m_isRunning{false};
  std::atomic<PlaybackState>    m_playbackState{PlaybackState::Stopped};
  std::thread                   m_audioThread;
  mutable std::mutex            m_mutex;

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

  void decodeAndPlay();
  void decodeStep(Sound& s);

  void cleanup()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sound.reset();
  }
};

} // namespace audio
