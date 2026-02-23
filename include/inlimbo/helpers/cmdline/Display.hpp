#pragma once

#include "InLimbo-Types.hpp"
#include "audio/Registry.hpp"
#include "audio/backend/Devices.hpp"
#include "telemetry/Context.hpp"
#include <optional>

namespace helpers::cmdline
{

void printFrontendPlugins();

void printAudioBackends(const audio::BackendList& backends);

void printAudioDevices(audio::Devices& devices);

void printArtists(const TS_SongMap& safeMap);

void printSongInfoByTitle(const TS_SongMap& safeMap, const std::optional<Title>& songName);

void printSongInfoByTitleAndArtist(const TS_SongMap& safeMap, const std::optional<Title>& songName,
                                   const std::optional<Artist>& artistName);

void printSongLyrics(const TS_SongMap& safeMap, const Title& songTitle);

void printSongTree(const TS_SongMap& safeMap, const std::optional<Artist>& artist);

void printAlbums(const TS_SongMap& safeMap, const std::optional<Artist>& artist);

void printGenres(const TS_SongMap& safeMap, const std::optional<Artist>& artist);

void printSongsByArtist(const TS_SongMap& safeMap, const Artist& artist);

void printSongsByAlbum(const TS_SongMap& safeMap, const Album& album);

void printSongsByGenre(const TS_SongMap& safeMap, const Genre& genre);

void printSongPaths(const TS_SongMap& safeMap);

void printSummary(const TS_SongMap& safeMap, const telemetry::Context& telemetryCtx);

} // namespace helpers::cmdline
