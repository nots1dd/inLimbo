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

// 1. audio::Service:
//
// It is the API by which frontends can interact with the core audio logic itself.
//
// Here is the design of the audio structure:
//
//                      audio::Service
//                            |
//                            |
//                            V
//                  audio::backend::IAudioBackend          (interacts with a base interface class)
//                            |
//                            |
//                            V
//                  audio::backend::AlsaBackend, etc.
//        (core logic of each audio backend written as child of interface)
//
// The available backends are enumerated at compile time but choosing the backend can be done via
// config.
//
// The Service has a ctor that will read the config (and fallback if none) and make the audio
// backend. If it doesnt work (due to invalid fallback or any error) it will not run the
// application!
//
// The API is simple enough and quite self explanatory with the exception of playlists.
//
// 1.1 audio::service::Playlist
//
// This is currently a misnomer on my part, this is not really a playlist that people imagine it to
// be. It is simply the list (or queue) of next/previous items to be played (check
// include/inlimbo/audio/Playlist.hpp)
//
// It allows for the following:
//
// 1. Operates as a pretty decent queue (can enumerate it, delete/add items to it)
// 2. Can jump to random index in the playlist itself
//
// In the future, we can extend this logic to have multiple "playlists" that will act the same
// except that each playlist will have different songs queued in them.
//
// Ex: Top Listened to Songs Playlist:
// Since we have basic telemetry that can gather the top X listened to songs, we simply register
// them to a new playlist and thats it! We just need to have the logic to "swap" between playlists
// rather than overwriting `m_playlist` member in audio::Service.
//
// Anyway that is planned for later but is not implemented yet.
//
// 1.2 audio::Devices:
//
// Here "devices" mean nodes that can play audio output. Now the terminology does defer between
// different audio backend API (like ex: ALSA it only refers to actuall hardware device sinks, but
// pipewire refers to any output - even bluetooth as a device node)
//
// I prefer pipewire's meaning of a device so the codebase does not mention it to be hardware
// specific.
//
// 1.3 audio::BackendInfo:
//
// This is the interfacing struct for the frontends to fetch details on the audio backend in real
// time.
//
// It is split into 2 components:
//
// 1. Common
// 2. Specific
//
// Common backend info is well... common. It has primitive dtypes and members that represent the
// general characteristics of any audio backend (ex: sample rate, channels, xruns, writes, etc.)
//
// Check out include/inlimbo/audio/backend/Backend.hpp for more details.
//
// Specific backend info is implemented as `std::variant` of different implemented backends and can
// house backend specific information like `snd_pcm_uframes_t` to get the buffer / period size,
// `snd_pcm_format_t` and so on
//
// Displaying the logic is easy enough just use `std::visit` and check for the currently implemented
// backends in the codebase. (FTXUI frontend has this covered check out the code there)
//
// 1.4 audio::TrackTable and audio::SoundHandle;
//
// Each song in playlist and in audio backends are referenced not by the whole song obj but by their
// `SoundHandle`. This saves needless copying and gruesome object lifetime management in multiple
// places, and there is a singular place to make the `SoundHandle` -> std::shared_ptr<const Song>
// relation - in audio::TrackTable.
//
// It is a simple unordered map that maps each sound handle to the respective shared pointer of the
// Song obj.
//
// This also has a side benefit of not invoking too many `query::songmap` commands to fetch the
// right object.
//
// 2. Future
//
// As you can see there are few public interfacing methods like `copySequence`, `withAudioBuffer` so
// on.
//
// The goal for that was to be able to safely share the current resampled audio buffers from the
// backend in real time to anybody that needs it.
//
// I wrote this keeping audio visualization in mind like so:
//
//                                  withAudioBuffer
//                                        |
//                                        |
//                                        V
//                              FFTW3 AudioVisualizer class
//                                        |
//                                        |
//                                        V
//                                do cool AV stuff            (via copySequence, copyBufferSize)
//
// It is not of a top priority right now but this is slated for the future.

namespace audio
{

class Service final
{
public:
  Service(threads::SafeMap<SongMap>& songMap, const std::string& backendName);
  ~Service();

  IMMUTABLE(Service);

  auto               enumerateDevices() -> Devices;
  void               initForDevice(const DeviceName& deviceName = "default");
  void               switchDevice(const DeviceName& deviceName);
  [[nodiscard]] auto getCurrentDevice() -> DeviceName;
  auto               getBackendInfo() -> backend::BackendInfo;

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
