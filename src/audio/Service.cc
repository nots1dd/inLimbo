#include "audio/Service.hpp"
#include "Logger.hpp"
#include "audio/backend/alsa/Impl.hpp"
#include "utils/string/Equals.hpp"
#include <random>

namespace audio
{

Service::Service(threads::SafeMap<SongMap>& songMapTS, const std::string& backendName)
    : m_songMapTS(songMapTS)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  if (utils::string::isEquals(backendName, "alsa"))
  {
    LOG_DEBUG("Found ALSA backend. Creating audio service...");
    m_backend = std::make_shared<backend::AlsaBackend>();
    LOG_INFO("Audio Backend '{}' (ID: {}) created.", m_backend->backendString(),
             (int)m_backend->backendID());
  }
}

Service::~Service() { shutdown(); }

auto Service::enumerateDevices() -> Devices
{
  return withBackend([](IAudioBackend& b) -> Devices { return b.enumerateDevices(); });
}

void Service::initForDevice(const DeviceName& deviceName)
{
  withBackend([&](IAudioBackend& b) -> void { b.initForDevice(deviceName); });
}

void Service::switchDevice(const DeviceName& deviceName)
{
  withBackend([&](IAudioBackend& b) -> void { b.switchDevice(deviceName); });
}

auto Service::getCurrentDevice() -> DeviceName
{
  return withBackend([](IAudioBackend& b) -> DeviceName { return b.currentDevice(); });
}

auto Service::isPlaying() -> bool
{
  return withBackend([](IAudioBackend& b) -> bool { return b.state() == PlaybackState::Playing; });
}

auto Service::isTrackFinished() -> bool
{
  return withBackend([](IAudioBackend& b) -> bool { return b.isTrackFinished(); });
}

void Service::clearTrackFinishedFlag()
{
  withBackend([](IAudioBackend& b) -> void { b.clearTrackFinished(); });
}

auto Service::getBackendInfo() -> backend::BackendInfo
{
  return withBackend([](IAudioBackend& b) -> backend::BackendInfo { return b.getBackendInfo(); });
}

auto Service::getPlaybackTime() -> std::optional<std::pair<double, double>>
{
  return withBackend([](IAudioBackend& b) -> auto { return b.playbackTime(); });
}

auto Service::registerTrack(std::shared_ptr<const Song> song) -> service::SoundHandle
{
  std::lock_guard<std::mutex> lock(m_mutex);
  service::SoundHandle        h{m_nextTrackId++};
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
  std::shared_ptr<IAudioBackend> backend;

  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (index >= m_playlist.tracks.size())
      return;

    const bool removingCurrent = (index == m_playlist.current);
    m_playlist.removeAt(index);

    if (!removingCurrent)
      return;

    backend = m_backend;
  }

  backend->stop();

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_playlist.empty())
      loadSoundUnlocked();
  }

  backend->play();
}

void Service::clearPlaylist()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_playlist.clear();
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
  std::shared_ptr<IAudioBackend> backend;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    loadSoundUnlocked();
    backend = m_backend;
  }

  backend->play();
}

void Service::playCurrent()
{
  withBackend([](IAudioBackend& b) -> void { b.play(); });
}

void Service::pauseCurrent()
{
  withBackend([](IAudioBackend& b) -> void { b.pause(); });
}

void Service::restart()
{
  std::shared_ptr<IAudioBackend> backend;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    ensureEngine();
    loadSoundUnlocked();
    backend = m_backend;
  }

  backend->restart();
}

void Service::restartCurrent()
{
  withBackend([](IAudioBackend& b) -> void { b.restart(); });
}

auto Service::nextTrack() -> std::optional<service::SoundHandle>
{
  std::shared_ptr<IAudioBackend>      backend;
  std::optional<service::SoundHandle> h;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    h = m_playlist.next();
    if (!h)
      return std::nullopt;
    loadSoundUnlocked();
    backend = m_backend;
  }

  backend->play();
  return h;
}

auto Service::nextTrackGapless() -> std::optional<service::SoundHandle>
{
  std::optional<service::SoundHandle> h;
  Path                                path;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    h = m_playlist.next();
    if (!h)
      return std::nullopt;

    auto it = m_trackTable.find(h->id);
    if (it == m_trackTable.end() || !it->second)
      throw std::runtime_error("invalid track handle");

    path = it->second->metadata.filePath.c_str();
  }

  withBackend(
    [&](IAudioBackend& b) -> void
    {
      if (!b.queueNext(path.c_str()))
        throw std::runtime_error("failed to queue next sound");
    });

  return h;
}

auto Service::previousTrack() -> std::optional<service::SoundHandle>
{
  std::shared_ptr<IAudioBackend>      backend;
  std::optional<service::SoundHandle> h;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    h = m_playlist.previous();
    if (!h)
      return std::nullopt;
    loadSoundUnlocked();
    backend = m_backend;
  }

  backend->play();
  return h;
}

auto Service::randomTrack() -> std::optional<service::SoundHandle>
{
  std::shared_ptr<IAudioBackend>      backend;
  std::optional<service::SoundHandle> h;
  Path                                path;

  {
    std::lock_guard<std::mutex> lock(m_mutex);

    ensureEngine();

    h = m_playlist.jumpToRandom();
    if (!h)
      return std::nullopt;

    auto it = m_trackTable.find(h->id);
    if (it == m_trackTable.end() || it->second == nullptr)
      throw std::runtime_error("audio::Service: invalid track handle");

    path    = it->second->metadata.filePath.c_str();
    backend = m_backend;
  }

  backend->stop();

  if (!backend->load(path.c_str()))
    throw std::runtime_error("audio::Service: failed to load sound!");

  backend->play();
  return h;
}

void Service::seekToAbsolute(double seconds)
{
  withBackend([&](IAudioBackend& b) -> void { b.seekAbsolute(seconds); });
}

void Service::seekForward(double seconds)
{
  withBackend([&](IAudioBackend& b) -> void { b.seekForward(seconds); });
}

void Service::seekBackward(double seconds)
{
  withBackend([&](IAudioBackend& b) -> void { b.seekBackward(seconds); });
}

void Service::setVolume(float v)
{
  withBackend([&](IAudioBackend& b) -> void { b.setVolume(v); });
}

auto Service::getVolume() -> float
{
  return withBackend([](IAudioBackend& b) -> float { return b.volume(); });
}

auto Service::getCurrentTrackInfo() -> std::optional<service::TrackInfo>
{
  backend::BackendInfo backend;
  auto                 timeOpt = withBackend(
    [&](IAudioBackend& b) -> auto
    {
      backend = b.getBackendInfo();
      return b.playbackTime();
    });

  if (!timeOpt)
    return std::nullopt;

  auto [pos, len] = *timeOpt;

  service::TrackInfo info;
  info.positionSec = pos;
  info.lengthSec   = len;
  info.playing     = backend.common.isPlaying;
  info.sampleRate  = backend.common.sampleRate;
  info.channels    = backend.common.channels;

  if (auto* alsa = std::get_if<backend::AlsaBackendInfo>(&backend.specific))
    info.format = alsa->pcmFormatName;
  else
    info.format = "<unknown>";

  static std::atomic<ui8> tidCounter{static_cast<ui8>(std::random_device{}())};
  info.tid = tidCounter.fetch_add(1, std::memory_order_relaxed);

  return info;
}

auto Service::getCurrentMetadata() -> std::optional<Metadata>
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto                        h = m_playlist.currentTrack();
  if (!h)
    return std::nullopt;

  auto it = m_trackTable.find(h->id);
  if (it == m_trackTable.end() || !it->second)
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
  std::shared_ptr<IAudioBackend> backend;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    backend = std::move(m_backend);
  }

  if (backend)
    backend->stop();
}

void Service::ensureEngine()
{
  if (!m_backend)
    throw std::runtime_error("audio::Service: backend not initialized");
}

void Service::loadSoundUnlocked()
{
  if (m_playlist.empty())
    return;

  auto h = m_playlist.currentTrack();
  if (!h)
    return;

  auto it = m_trackTable.find(h->id);
  if (it == m_trackTable.end() || !it->second)
    throw std::runtime_error("invalid track handle");

  const auto& path = it->second->metadata.filePath;

  auto backend = m_backend; // safe: lock already held
  backend->stop();

  if (!backend->load(path.c_str()))
    throw std::runtime_error("failed to load sound");
}

auto Service::copySequence() -> ui64
{
  return withBackend([](IAudioBackend& b) -> ui64 { return b.copySequence(); });
}

auto Service::copyBufferSize() -> size_t
{
  return withBackend([](IAudioBackend& b) -> size_t { return b.copyBufferSize(); });
}

} // namespace audio
