#pragma once

#include "audio/Service.hpp"
#include "telemetry/Context.hpp"

namespace helpers::telemetry
{

void beginPlayback(audio::Service& audio, ::telemetry::Context* telemetryCtx,
                   std::optional<::telemetry::Event>& currentTelemetryEvent,
                   std::optional<i64>&                lastTick);

void updateTelemetryProgress(audio::Service&                    audio,
                             std::optional<::telemetry::Event>& currentTelemetryEvent,
                             std::optional<i64>&                lastTick);

void endPlayback(audio::Service& audio, ::telemetry::Context* telemetryCtx,
                 std::optional<::telemetry::Event>& currentTelemetryEvent,
                 std::optional<i64>&                lastTick);

template <typename Fn>
inline void playbackTransition(audio::Service& audio, ::telemetry::Context* telemetryCtx,
                               std::optional<::telemetry::Event>& currentTelemetryEvent,
                               std::optional<i64>& lastTick, Fn&& customFn)
{
  helpers::telemetry::endPlayback(audio, telemetryCtx, currentTelemetryEvent, lastTick);

  if constexpr (!std::is_same_v<std::decay_t<Fn>, std::nullptr_t>)
    customFn();

  helpers::telemetry::beginPlayback(audio, telemetryCtx, currentTelemetryEvent, lastTick);
}

} // namespace helpers::telemetry
