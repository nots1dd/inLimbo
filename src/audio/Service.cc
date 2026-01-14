#include "audio/Service.hpp"

namespace audio
{

Service::Service()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_engine = std::make_shared<AudioEngine>();
}

Service::~Service()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  shutdownLocked();
}

auto Service::enumeratePlaybackDevices() -> Devices
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  return m_engine->enumeratePlaybackDevices();
}

void Service::initDevice(const DeviceName& deviceName)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  m_engine->initEngineForDevice(deviceName);
}

auto Service::isPlaying() -> bool
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  return m_engine->isPlaying();
}

auto Service::getBackendInfo() -> BackendInfo
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  return m_engine->getBackendInfo();
}

auto Service::registerTrack(const Song& song) -> service::SoundHandle
{
  std::lock_guard<std::mutex> lock(m_mutex);

  service::SoundHandle h{m_nextTrackId++};
  m_trackTable.emplace(h.id, song.metadata.filePath);
  m_metadataTable.emplace(h.id, song.metadata);

  return h;
}

void Service::addToPlaylist(service::SoundHandle h)
{
  if (!h)
    return;

  std::lock_guard<std::mutex> lock(m_mutex);
  m_playlist.tracks.push_back(h);
}

void Service::clearPlaylist()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_playlist.clear();
}

auto Service::getPlaybackTime() -> std::optional<std::pair<double, double>>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  return m_engine->getPlaybackTime();
}

auto Service::getCurrentTrack() -> service::SoundHandle
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_playlist.currentTrack();
}

auto Service::getCurrentIndex() -> size_t
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_playlist.current;
}

auto Service::getPlaylistSize() -> size_t
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_playlist.tracks.size();
}

void Service::start()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  loadSound();
  m_engine->play();
}

void Service::playCurrent()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  m_engine->play();
}

void Service::pauseCurrent()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  m_engine->pause();
}

auto Service::nextTrack() -> service::SoundHandle
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

auto Service::previousTrack() -> service::SoundHandle
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

void Service::restartCurrent()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  m_engine->restart();
}

void Service::seekToAbsolute(double seconds)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_engine->seekToAbsolute(seconds);
}

void Service::seekForward(double seconds)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_engine->seekForward(seconds);
}

void Service::seekBackward(double seconds)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_engine->seekBackward(seconds);
}

void Service::setVolume(float v)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_engine->setVolume(v);
}

auto Service::getVolume() -> float
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_engine->getVolume();
}

auto Service::getCurrentTrackInfo() -> std::optional<service::TrackInfo>
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

auto Service::getCurrentMetadata() -> std::optional<Metadata>
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

auto Service::getMetadataAt(size_t index) -> std::optional<Metadata>
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (index >= m_playlist.tracks.size())
    return std::nullopt;

  const auto h  = m_playlist.tracks[index];
  auto       it = m_metadataTable.find(h.id);
  if (it == m_metadataTable.end())
    return std::nullopt;

  return it->second;
}

void Service::shutdown()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  shutdownLocked();
}

void Service::ensureEngine()
{
  if (!m_engine)
    throw std::runtime_error("Service: engine not initialized");
}

void Service::loadSound()
{
  if (m_playlist.empty())
    return;

  const auto h  = m_playlist.currentTrack();
  auto       it = m_trackTable.find(h.id);
  if (it == m_trackTable.end())
    throw std::runtime_error("Service: invalid track handle");

  m_engine->stop();

  if (!m_engine->loadSound(it->second))
    throw std::runtime_error("Service: failed to load sound");
}

void Service::shutdownLocked()
{
  if (!m_engine)
    return;

  m_engine->stop();
  m_engine.reset();
}

} // namespace audio
