#pragma once

#include "Config.hpp"
#include "Sound.hpp"

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

struct Device
{
  std::string name;
  std::string description;
  int         cardIndex   = -1;
  int         deviceIndex = -1;
  bool        isDefault   = false;
};

struct BackendInfo
{
  Device           dev;
  int              sampleRate    = DEFAULT_SOUND_SAMPLE_RATE;
  int              channels      = DEFAULT_SOUND_CHANNELS;
  snd_pcm_format_t pcmFormat     = SND_PCM_FORMAT_FLOAT_LE;
  std::string      pcmFormatName = {};
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
  INLIMBO_API_CPP void initEngineForDevice(const std::string& deviceName = "default");

  INLIMBO_API_CPP auto loadSound(const std::string& path) -> std::optional<size_t>;

  INLIMBO_API_CPP void play(size_t index = 0);
  INLIMBO_API_CPP void pause(size_t index = 0);
  INLIMBO_API_CPP void stop(size_t index = 0);

  INLIMBO_API_CPP void restart(size_t i = 0)
  {
    seekTo(0.0, i);
    play(i);
  }

  INLIMBO_API_CPP [[nodiscard]] auto isPlaying(size_t index = 0) const -> bool
  {
    return index < m_sounds.size() && m_playbackState == PlaybackState::Playing;
  }

  INLIMBO_API_CPP auto getPlaybackTime(size_t i = 0) const
    -> std::optional<std::pair<double, double>>;

  INLIMBO_API_CPP void seekTo(double seconds, size_t i = 0);
  INLIMBO_API_CPP void seekForward(double seconds, size_t i = 0);
  INLIMBO_API_CPP void seekBackward(double seconds, size_t i = 0);

  INLIMBO_API_CPP void setVolume(float v)
  {
    m_volume.store(std::clamp(v, 0.0f, 1.5f));
  }

  INLIMBO_API_CPP auto getBackendInfo() const -> BackendInfo
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_backendInfo;
  }

  INLIMBO_API_CPP [[nodiscard]] auto getVolume(size_t index = 0) const -> float
  {
    if (index < m_sounds.size())
      return m_volume.load();
    return 0.0f;
  }

  INLIMBO_API_CPP [[nodiscard]] auto getSoundCount() const -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sounds.size();
  }

  INLIMBO_API_CPP [[nodiscard]] auto getCurrentDevice() const -> std::string
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentDevice;
  }

  INLIMBO_API_CPP void unloadSound(size_t index)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (index < m_sounds.size())
    {
      m_sounds.erase(m_sounds.begin() + index);
    }
  }

private:
  snd_pcm_t*  m_pcmData = nullptr;
  BackendInfo m_backendInfo;
  std::string m_currentDevice = "default";

  Sounds_ptr                 m_sounds;
  std::atomic<float>         m_volume{1.0f};
  std::atomic<bool>          m_isRunning{false};
  std::atomic<PlaybackState> m_playbackState{PlaybackState::Stopped};
  std::thread                m_audioThread;
  mutable std::mutex         m_mutex;

  void startThread()
  {
    if (m_isRunning)
      return;
    m_isRunning   = true;
    m_audioThread = std::thread(&AudioEngine::audioLoop, this);
  }

  void initAlsa(const std::string& deviceName = "default");
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
    m_sounds.clear();
  }
};

} // namespace audio
