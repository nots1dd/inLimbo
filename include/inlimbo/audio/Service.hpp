#pragma once

#include "Engine.hpp"
#include "Playlist.hpp"

#include <memory>
#include <mutex>
#include <optional>

namespace audio
{

class Service final
{
public:
  Service();
  ~Service();

  Service(const Service&)                    = delete;
  auto operator=(const Service&) -> Service& = delete;
  Service(Service&&)                         = delete;
  auto operator=(Service&&) -> Service&      = delete;

  auto enumeratePlaybackDevices() -> Devices;
  void initDevice(const DeviceName& deviceName = "default");
  auto getBackendInfo() -> BackendInfo;

  auto registerTrack(const Song& song) -> service::SoundHandle;

  void addToPlaylist(service::SoundHandle h);
  void clearPlaylist();
  auto getCurrentTrack() -> service::SoundHandle;
  auto getCurrentIndex() -> size_t;
  auto getPlaylistSize() -> size_t;

  void start();
  void playCurrent();
  void pauseCurrent();
  auto nextTrack() -> service::SoundHandle;
  auto previousTrack() -> service::SoundHandle;
  void restartCurrent();

  void seekForward(double seconds);
  void seekBackward(double seconds);

  void setVolume(float v);
  auto getVolume() -> float;

  auto getCurrentTrackInfo() -> std::optional<service::TrackInfo>;
  auto getCurrentMetadata() -> std::optional<Metadata>;
  auto getMetadataAt(size_t index) -> std::optional<Metadata>;

  void shutdown();

private:
  std::shared_ptr<AudioEngine> m_engine;
  service::Playlist            m_playlist;

  service::TrackTable    m_trackTable;
  service::MetadataTable m_metadataTable;
  ui64                   m_nextTrackId = 1;

  std::mutex m_mutex;

  void ensureEngine();
  void loadSound();
  void shutdownLocked();
};

} // namespace audio
