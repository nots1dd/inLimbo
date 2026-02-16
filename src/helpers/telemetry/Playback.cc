#include "helpers/telemetry/Playback.hpp"
#include "Logger.hpp"
#include "utils/timer/Timer.hpp"

namespace helpers::telemetry
{

void beginPlayback(::audio::Service& audio, ::telemetry::Context* telemetryCtx,
                   std::optional<::telemetry::Event>& currentTelemetryEvent,
                   std::optional<i64>&                lastTick)
{
  auto metaOpt = audio.getCurrentMetadata();
  if (!metaOpt)
  {
    LOG_WARN("No metadata available for current track, skipping telemetry event");
    return;
  }

  ::telemetry::Event ev{};
  ev.type   = ::telemetry::EventType::PlayEnd;
  ev.song   = telemetryCtx->registry.titles.getOrCreate(metaOpt->title);
  ev.artist = telemetryCtx->registry.artists.getOrCreate(metaOpt->artist);
  telemetryCtx->registry.bindSongToArtist(ev.song, ev.artist);
  ev.album     = telemetryCtx->registry.albums.getOrCreate(metaOpt->album);
  ev.genre     = telemetryCtx->registry.genres.getOrCreate(metaOpt->genre);
  ev.seconds   = 0.0;
  ev.timestamp = utils::timer::nowUnix();

  currentTelemetryEvent = ev;
  lastTick              = ev.timestamp;

  LOG_DEBUG("Started playback telemetry event: {} - {}",
            *telemetryCtx->registry.artists.toString(ev.artist),
            *telemetryCtx->registry.titles.toString(ev.song));
}

void updateTelemetryProgress(::audio::Service&                  audio,
                             std::optional<::telemetry::Event>& currentTelemetryEvent,
                             std::optional<i64>&                lastTick)

{
  if (!currentTelemetryEvent || !lastTick)
  {
    LOG_DEBUG("No active telemetry event or last tick, skipping progress update");
    return;
  }

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt || !infoOpt->playing)
    return;

  const auto now = utils::timer::nowUnix();
  const auto dt  = now - *lastTick;

  if (dt > 0)
    currentTelemetryEvent->seconds += double(dt);

  lastTick = now;
}

void endPlayback(::audio::Service& audio, ::telemetry::Context* telemetryCtx,
                 std::optional<::telemetry::Event>& currentTelemetryEvent,
                 std::optional<i64>&                lastTick)
{
  if (!currentTelemetryEvent)
  {
    LOG_WARN("Attempted to end playback telemetry event, but no active event found");
    return;
  }

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
  {
    LOG_WARN("Attempted to end playback telemetry event, but no track info available");
    return;
  }

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

  LOG_DEBUG("Ended playback telemetry event for track: {} - {}",
            *telemetryCtx->registry.artists.toString(currentTelemetryEvent->artist),
            *telemetryCtx->registry.titles.toString(currentTelemetryEvent->song));
}

} // namespace helpers::telemetry
