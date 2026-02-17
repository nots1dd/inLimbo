#pragma once

#include "InLimbo-Types.hpp"
#include "Playlist.hpp"
#include "audio/backend/Devices.hpp"
#include "audio/backend/Interface.hpp"
#include "thread/Map.hpp"
#include "utils/ClassRulesMacros.hpp"

#include <memory>
#include <mutex>
#include <optional>

// Audio Service is intended as a high-level interface for the
// not thread-safe AudioEngine, while managing a playlist and track registry.
//
// It provides thread-safe access to playback controls, track management,
// and device enumeration/initialization.

namespace audio
{

class Service final
{
public:
  Service(threads::SafeMap<SongMap>& songMap);
  ~Service();

  IMMUTABLE(Service);

  auto               enumerateDevices() -> Devices;
  void               initForDevice(const DeviceName& deviceName = "default");
  void               switchDevice(const DeviceName& deviceName);
  [[nodiscard]] auto getCurrentDevice() -> DeviceName;
  auto               getBackendInfo() -> BackendInfo;

  auto registerTrack(std::shared_ptr<const Song> song) -> service::SoundHandle;
  auto isPlaying() -> bool;

  auto getPlaybackTime() -> std::optional<std::pair<double, double>>;
  auto getCurrentTrack() -> std::optional<service::SoundHandle>;
  auto getCurrentIndex() -> size_t;
  auto getPlaylistSize() -> size_t;

  void start();
  void playCurrent();
  void pauseCurrent();
  auto isTrackFinished() -> bool;
  void clearTrackFinishedFlag();
  auto nextTrack() -> std::optional<service::SoundHandle>;
  auto previousTrack() -> std::optional<service::SoundHandle>;
  auto nextTrackGapless() -> std::optional<service::SoundHandle>;
  auto previousTrackGapless() -> std::optional<service::SoundHandle>;
  void restartCurrent();
  void restart();

  // playlist stuff
  auto randomTrack() -> std::optional<service::SoundHandle>;
  auto randomIndex() -> std::optional<size_t>;
  void addToPlaylist(service::SoundHandle h);
  void removeFromPlaylist(size_t index);
  void clearPlaylist();

  // AV stuff
  template <typename Fn>
  auto withAudioBuffer(Fn&& fn) -> void;
  auto copySequence() -> ui64;
  auto copyBufferSize() -> size_t;

  void seekToAbsolute(double seconds);
  void seekForward(double seconds);
  void seekBackward(double seconds);

  void setVolume(float v);
  auto getVolume() -> float;

  auto getCurrentTrackInfo() -> std::optional<service::TrackInfo>;
  auto getCurrentMetadata() -> std::optional<Metadata>;
  auto getMetadataAt(size_t index) -> std::optional<Metadata>;

  void shutdown();

private:
  std::shared_ptr<IAudioBackend> m_backend;
  service::Playlist              m_playlist;
  threads::SafeMap<SongMap>&     m_songMapTS;

  service::TrackTable m_trackTable;
  ui64                m_nextTrackId = 1;

  std::mutex m_mutex;

  template <typename Fn>
  auto withBackend(Fn&& fn);

  void ensureEngine();
  // void loadSound();
  void loadSoundUnlocked();
  void shutdownLocked();
};

template <typename Fn>
auto Service::withBackend(Fn&& fn)
{
  std::shared_ptr<IAudioBackend> backend;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    backend = m_backend;
  }
  return fn(*backend);
}

template <typename Fn>
auto Service::withAudioBuffer(Fn&& fn) -> void
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  m_backend->withAudioBuffer(fn);
}

} // namespace audio
