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

} // namespace helpers::telemetry
