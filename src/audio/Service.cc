#include "audio/Service.hpp"
#include "Logger.hpp"
#include <random>

namespace audio
{

Service::Service(threads::SafeMap<SongMap>& songMapTS) : m_songMapTS(songMapTS)
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

auto Service::registerTrack(std::shared_ptr<const Song> song) -> service::SoundHandle
{
  std::lock_guard<std::mutex> lock(m_mutex);

  service::SoundHandle h{m_nextTrackId++};
  m_trackTable.emplace(h.id, std::move(song));

  return h;
}

void Service::addToPlaylist(service::SoundHandle h)
{
  if (!h)
    return;

  std::lock_guard<std::mutex> lock(m_mutex);
  m_playlist.tracks.push_back(h);
}

void Service::removeFromPlaylist(size_t index)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (index >= m_playlist.tracks.size())
    return;

  const bool removingCurrent = (index == m_playlist.current);

  m_playlist.removeAt(index);

  if (removingCurrent)
  {
    // Stop current playback
    m_engine->stop();

    // Load next valid track if any
    if (!m_playlist.empty())
    {
      loadSound();
      m_engine->play();
    }
  }
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

auto Service::getCurrentTrack() -> std::optional<service::SoundHandle>
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

auto Service::isTrackFinished() -> bool
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  return m_engine->isTrackFinished();
}

void Service::clearTrackFinishedFlag()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  m_engine->clearTrackFinishedFlag();
}

auto Service::nextTrack() -> std::optional<service::SoundHandle>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  auto h = m_playlist.next();
  if (!h)
    return std::nullopt;

  loadSound();
  m_engine->play();
  return h;
}

auto Service::nextTrackGapless() -> std::optional<service::SoundHandle>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  auto h = m_playlist.next();
  if (!h)
    return std::nullopt;

  auto it = m_trackTable.find(h->id);
  if (it == m_trackTable.end() || it->second == nullptr)
    throw std::runtime_error("audio::Service: invalid track handle (nextTrackGapless)");

  const auto& path = it->second->metadata.filePath;

  if (!m_engine->queueNextSound(path.c_str()))
    throw std::runtime_error("audio::Service: failed to queue next sound (gapless)");

  return h;
}

auto Service::previousTrack() -> std::optional<service::SoundHandle>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  auto h = m_playlist.previous();
  if (!h)
    return std::nullopt;

  loadSound();
  m_engine->play();
  return h;
}

auto Service::previousTrackGapless() -> std::optional<service::SoundHandle>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  auto h = m_playlist.previous();
  if (!h)
    return std::nullopt;

  auto it = m_trackTable.find(h->id);
  if (it == m_trackTable.end() || it->second == nullptr)
    throw std::runtime_error("audio::Service: invalid track handle (prevTrackGapless)");

  const auto& path = it->second->metadata.filePath;

  if (!m_engine->queueNextSound(path.c_str()))
    throw std::runtime_error("audio::Service: failed to queue next sound (gapless)");

  return h;
}

void Service::restart()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  loadSound();
  m_engine->restart();
}

void Service::restartCurrent()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();
  m_engine->restart();
}

auto Service::randomIndex() -> std::optional<size_t>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_playlist.randomIndex();
}

auto Service::randomTrack() -> std::optional<service::SoundHandle>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  auto h = m_playlist.jumpToRandom();
  if (!h)
    return std::nullopt;

  LOG_TRACE("audio::Service: random track selected id={}", h->id);

  loadSound();
  m_engine->play();
  return h;
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

  static std::atomic<ui8> tidCounter{static_cast<ui8>(std::random_device{}())};

  info.tid = tidCounter.fetch_add(1, std::memory_order_relaxed);

  return info;
}

auto Service::getCurrentMetadata() -> std::optional<Metadata>
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto h = m_playlist.currentTrack();
  if (!h)
    return std::nullopt;

  auto it = m_trackTable.find(h->id);
  if (it == m_trackTable.end() || it->second == nullptr)
    return std::nullopt;

  return it->second->metadata;
}

auto Service::getMetadataAt(size_t index) -> std::optional<Metadata>
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (index >= m_playlist.tracks.size())
    return std::nullopt;

  const auto h = m_playlist.tracks[index];
  if (!h)
    return std::nullopt;

  auto it = m_trackTable.find(h.id);
  if (it == m_trackTable.end() || it->second == nullptr)
    return std::nullopt;

  return it->second->metadata;
}

void Service::shutdown()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  shutdownLocked();
}

void Service::ensureEngine()
{
  if (!m_engine)
    throw std::runtime_error("audio::Service: engine not initialized");
}

void Service::loadSound()
{
  if (m_playlist.empty())
    return;

  const auto h = m_playlist.currentTrack();
  if (!h)
    return;

  auto it = m_trackTable.find(h->id);
  if (it == m_trackTable.end() || it->second == nullptr)
    throw std::runtime_error("audio::Service: invalid track handle");

  const auto& path = it->second->metadata.filePath;

  m_engine->stop();

  if (!m_engine->loadSound(path.c_str()))
    throw std::runtime_error("audio::Service: failed to load sound!");
}

void Service::shutdownLocked()
{
  if (!m_engine)
    return;

  m_engine->stop();
  m_engine.reset();
}

auto Service::getCopySeq() -> ui64
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  return m_engine->getCopySeq();
}

auto Service::getCopyBufferSize() -> size_t
{
  std::lock_guard<std::mutex> lock(m_mutex);
  ensureEngine();

  return m_engine->getCopyBufferSize();
}

} // namespace audio
