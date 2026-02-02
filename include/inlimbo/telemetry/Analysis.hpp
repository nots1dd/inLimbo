#pragma once

#include "telemetry/Store.hpp"

namespace telemetry::analysis
{

auto recencyWeight(Timestamp last, Timestamp now) -> double;
auto mostPlayedSong(const Store& s) -> SongID;
auto hottestSong(const Store& s, Timestamp now) -> SongID;

} // namespace telemetry::analysis
