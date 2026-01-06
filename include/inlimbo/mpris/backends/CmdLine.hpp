#pragma once

#include "audio/Service.hpp"
#include "mpris/Interface.hpp"

namespace mpris::cmdline
{

class Backend final : public mpris::IMprisBackend
{
public:
  explicit Backend(audio::Service& audio) : audio_(audio) {}

  /* Playback */
  void play() override { audio_.playCurrent(); }
  void pause() override { audio_.pauseCurrent(); }

  void stop() override
  {
    audio_.pauseCurrent();
    audio_.seekAbsolute(0.0);
  }

  void next() override { audio_.nextTrack(); }
  void previous() override { audio_.previousTrack(); }

  /* Seek */
  void seekSeconds(double s) override
  {
    if (s > 0)
      audio_.seekForward(s);
    else
      audio_.seekBackward(-s);
  }

  void setPositionSeconds(double s) override { audio_.seekAbsolute(s); }

  /* State */
  [[nodiscard]] auto isPlaying() const -> bool override { return audio_.isPlaying(); }

  [[nodiscard]] auto positionSeconds() const -> double override
  {
    return audio_.getPlaybackTime().value_or(std::make_pair(0.0, 0.0)).first;
  }

  [[nodiscard]] auto durationSeconds() const -> double override
  {
    if (auto info = audio_.getCurrentTrackInfo())
      return info->lengthSec;
    return 0.0;
  }

  /* Metadata */
  [[nodiscard]] auto title() const -> Title override
  {
    auto m = audio_.getCurrentMetadata();
    return m ? m->title : "";
  }

  [[nodiscard]] auto artist() const -> Artist override
  {
    auto m = audio_.getCurrentMetadata();
    return m ? m->artist : "";
  }

  [[nodiscard]] auto album() const -> Album override
  {
    auto m = audio_.getCurrentMetadata();
    return m ? m->album : "";
  }

  [[nodiscard]] auto artUrl() const -> std::string override
  {
    auto m = audio_.getCurrentMetadata();
    return m ? m->artUrl : "";
  }

  /* Volume */
  [[nodiscard]] auto volume() const -> double override { return audio_.getVolume(); }
  void               setVolume(double v) override { audio_.setVolume(v); }

private:
  audio::Service& audio_;
};

} // namespace mpris::cmdline
