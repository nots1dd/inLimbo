#include "helpers/telemetry/Playback.hpp"
#include "utils/timer/Timer.hpp"

namespace helpers::telemetry
{

void beginPlayback(audio::Service& audio, ::telemetry::Context* telemetryCtx,
                   std::optional<::telemetry::Event>& currentTelemetryEvent,
                   std::optional<i64>&                lastTick)
{
  auto metaOpt = audio.getCurrentMetadata();
  if (!metaOpt)
    return;

  ::telemetry::Event ev{};
  ev.type      = ::telemetry::EventType::PlayEnd;
  ev.song      = telemetryCtx->registry.titles.getOrCreate(metaOpt->title);
  ev.artist    = telemetryCtx->registry.artists.getOrCreate(metaOpt->artist);
  ev.album     = telemetryCtx->registry.albums.getOrCreate(metaOpt->album);
  ev.genre     = telemetryCtx->registry.genres.getOrCreate(metaOpt->genre);
  ev.seconds   = 0.0;
  ev.timestamp = utils::timer::nowUnix();

  currentTelemetryEvent = ev;
  lastTick              = ev.timestamp;
}

void updateTelemetryProgress(audio::Service&                    audio,
                             std::optional<::telemetry::Event>& currentTelemetryEvent,
                             std::optional<i64>&                lastTick)

{
  if (!currentTelemetryEvent || !lastTick)
    return;

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt || !infoOpt->playing)
    return;

  const auto now = utils::timer::nowUnix();
  const auto dt  = now - *lastTick;

  if (dt > 0)
    currentTelemetryEvent->seconds += double(dt);

  lastTick = now;
}

void endPlayback(audio::Service& audio, ::telemetry::Context* telemetryCtx,
                 std::optional<::telemetry::Event>& currentTelemetryEvent,
                 std::optional<i64>&                lastTick)
{
  if (!currentTelemetryEvent)
    return;

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
    return;

  // Ignore zero-length or accidental triggers
  if (infoOpt->positionSec < 1.0)
  {
    currentTelemetryEvent.reset();
    lastTick.reset();
    return;
  }

  currentTelemetryEvent->timestamp = utils::timer::nowUnix();
  telemetryCtx->store.onEvent(*currentTelemetryEvent);

  currentTelemetryEvent.reset();
  lastTick.reset();
}

} // namespace helpers::telemetry
