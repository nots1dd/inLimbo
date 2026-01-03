#pragma once

#include "Engine.hpp"
#include "Playlist.hpp"
#include "core/SongTree.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace audio
{

class AudioService final
{
public:
  AudioService()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_engine = std::make_shared<AudioEngine>();
  }

  ~AudioService()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    shutdownLocked();
  }

  AudioService(const AudioService&)                    = delete;
  auto operator=(const AudioService&) -> AudioService& = delete;
  AudioService(AudioService&&)                         = delete;
  auto operator=(AudioService&&) -> AudioService&      = delete;

  // ------------------------------------------------------------
  // Backend / device
  // ------------------------------------------------------------
  auto enumeratePlaybackDevices() -> Devices
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    return m_engine->enumeratePlaybackDevices();
  }

  void initDevice(const std::string& deviceName = "default")
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    m_engine->initEngineForDevice(deviceName);
  }

  auto getBackendInfo() -> BackendInfo
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    return m_engine->getBackendInfo();
  }

  // ------------------------------------------------------------
  // Track registration (NO audio loaded here)
  // ------------------------------------------------------------
  auto registerTrack(const core::Song& song) -> service::SoundHandle
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    service::SoundHandle h{m_nextTrackId++};
    m_trackTable.emplace(h.id, song.metadata.filePath);

    m_metadataTable.emplace(h.id, song.metadata);

    return h;
  }

  // ------------------------------------------------------------
  // Playlist management
  // ------------------------------------------------------------
  void addToPlaylist(service::SoundHandle h)
  {
    if (!h)
      return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_playlist.tracks.push_back(h);
  }

  void clearPlaylist()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_playlist.clear();
  }

  auto getCurrentTrack() -> service::SoundHandle
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playlist.currentTrack();
  }

  auto getCurrentIndex() -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playlist.current;
  }

  auto getPlaylistSize() -> size_t
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_playlist.tracks.size();
  }

  // ------------------------------------------------------------
  // Playback (playlist-aware)
  // ------------------------------------------------------------
  void start()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    loadSound();
    m_engine->play();
  }

  void playCurrent()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    m_engine->play();
  }

  void pauseCurrent()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    m_engine->pause();
  }

  auto nextTrack() -> service::SoundHandle
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();

    auto h = m_playlist.next();
    if (!h)
      return {};

    loadSound();
    m_engine->play();
    return h;
  }

  void restartCurrent()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    m_engine->restart();
  }

  auto previousTrack() -> service::SoundHandle
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();

    auto h = m_playlist.previous();
    if (!h)
      return {};

    loadSound();
    m_engine->play();
    return h;
  }

  // ------------------------------------------------------------
  // Seeking
  // ------------------------------------------------------------
  void seekForward(double seconds)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_engine->seekForward(seconds);
  }

  void seekBackward(double seconds)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_engine->seekBackward(seconds);
  }

  // ------------------------------------------------------------
  // Volume
  // ------------------------------------------------------------
  void setVolume(float v)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_engine->setVolume(v);
  }

  auto getVolume() -> float
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_engine->getVolume();
  }

  // ------------------------------------------------------------
  // Track info snapshot
  // ------------------------------------------------------------
  auto getCurrentTrackInfo() -> std::optional<service::TrackInfo>
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();

    auto timeOpt = m_engine->getPlaybackTime();
    if (!timeOpt)
      return std::nullopt;

    auto [pos, len] = *timeOpt;

    service::TrackInfo info;
    info.positionSec = pos;
    info.lengthSec   = len;
    info.playing     = m_engine->isPlaying();

    const auto& backend = m_engine->getBackendInfo();
    info.sampleRate     = backend.sampleRate;
    info.channels       = backend.channels;
    info.format         = backend.pcmFormatName;

    return info;
  }

  auto getCurrentMetadata() -> std::optional<Metadata>
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto h = m_playlist.currentTrack();
    if (!h)
      return std::nullopt;

    auto it = m_metadataTable.find(h.id);
    if (it == m_metadataTable.end())
      return std::nullopt;

    return it->second;
  }

  auto getMetadataAt(size_t index) -> std::optional<Metadata>
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_playlist.tracks.empty())
      return std::nullopt;

    if (index >= m_playlist.tracks.size())
      return std::nullopt;

    const auto h = m_playlist.tracks[index];

    auto it = m_metadataTable.find(h.id);
    if (it == m_metadataTable.end())
      return std::nullopt;

    return it->second;
  }

  // ------------------------------------------------------------
  // Shutdown
  // ------------------------------------------------------------
  void shutdown()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    shutdownLocked();
  }

private:
  std::shared_ptr<AudioEngine> m_engine;
  service::Playlist            m_playlist;

  service::TrackTable    m_trackTable;
  service::MetadataTable m_metadataTable;
  ui64                   m_nextTrackId = 1;

  std::mutex m_mutex;

  void ensureEngine()
  {
    if (!m_engine)
      throw std::runtime_error("AudioService: engine not initialized");
  }

  void loadSound()
  {
    if (m_playlist.empty())
      return;

    const auto h  = m_playlist.currentTrack();
    auto       it = m_trackTable.find(h.id);
    if (it == m_trackTable.end())
      throw std::runtime_error("AudioService: invalid track handle");

    m_engine->stop();

    if (!m_engine->loadSound(it->second))
      throw std::runtime_error("AudioService: failed to load sound");
  }

  void shutdownLocked()
  {
    if (!m_engine)
      return;

    m_engine->stop();
    m_engine.reset();
  }
};

} // namespace audio
