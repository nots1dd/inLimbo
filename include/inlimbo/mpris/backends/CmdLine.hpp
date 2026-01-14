#pragma once

#include "audio/Service.hpp"
#include "mpris/Interface.hpp"

namespace mpris::cmdline
{

class Backend final : public mpris::IMprisBackend
{
public:
  explicit Backend(audio::Service& audio) : m_audioService(audio) {}

  /* Playback */
  void play() override { m_audioService.playCurrent(); }
  void pause() override { m_audioService.pauseCurrent(); }

  void stop() override
  {
    m_audioService.pauseCurrent();
    m_audioService.seekToAbsolute(0.0);
  }

  void next() override { m_audioService.nextTrack(); }
  void previous() override { m_audioService.previousTrack(); }

  /* Seek */
  void seekSeconds(double s) override
  {
    if (s > 0)
      m_audioService.seekForward(s);
    else
      m_audioService.seekBackward(-s);
  }

  void setPositionSeconds(double s) override { m_audioService.seekToAbsolute(s); }

  /* State */
  [[nodiscard]] auto isPlaying() const -> bool override { return m_audioService.isPlaying(); }

  [[nodiscard]] auto positionSeconds() const -> double override
  {
    return m_audioService.getPlaybackTime().value_or(std::make_pair(0.0, 0.0)).first;
  }

  [[nodiscard]] auto durationSeconds() const -> double override
  {
    if (auto info = m_audioService.getCurrentTrackInfo())
      return info->lengthSec;
    return 0.0;
  }

  /* Metadata */
  [[nodiscard]] auto title() const -> Title override
  {
    auto m = m_audioService.getCurrentMetadata();
    return m ? m->title : "";
  }

  [[nodiscard]] auto artist() const -> Artist override
  {
    auto m = m_audioService.getCurrentMetadata();
    return m ? m->artist : "";
  }

  [[nodiscard]] auto album() const -> Album override
  {
    auto m = m_audioService.getCurrentMetadata();
    return m ? m->album : "";
  }

  [[nodiscard]] auto artUrl() const -> std::string override
  {
    auto m = m_audioService.getCurrentMetadata();
    return m ? m->artUrl : "";
  }

  /* Volume */
  [[nodiscard]] auto volume() const -> double override { return m_audioService.getVolume(); }
  void               setVolume(double v) override { m_audioService.setVolume(v); }

private:
  audio::Service& m_audioService;
};

} // namespace mpris::cmdline
