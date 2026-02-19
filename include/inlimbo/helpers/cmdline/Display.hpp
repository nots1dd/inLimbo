#pragma once

#include "InLimbo-Types.hpp"
#include "audio/Registry.hpp"
#include "audio/backend/Devices.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"
#include <optional>

namespace helpers::cmdline
{

void printFrontendPlugins();

void printAudioBackends(const audio::BackendList& backends);

void printAudioDevices(audio::Devices& devices);

void printArtists(const threads::SafeMap<SongMap>& safeMap);

void printSongInfoByTitle(const threads::SafeMap<SongMap>& safeMap,
                          const std::optional<Title>&      songName);

void printSongInfoByTitleAndArtist(const threads::SafeMap<SongMap>& safeMap,
                                   const std::optional<Title>&      songName,
                                   const std::optional<Artist>&     artistName);

void printSongLyrics(const threads::SafeMap<SongMap>& safeMap, const Title& songTitle);

void printSongTree(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist);

void printAlbums(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist);

void printGenres(const threads::SafeMap<SongMap>& safeMap, const std::optional<Artist>& artist);

void printSongsByArtist(const threads::SafeMap<SongMap>& safeMap, const Artist& artist);

void printSongsByAlbum(const threads::SafeMap<SongMap>& safeMap, const Album& album);

void printSongsByGenre(const threads::SafeMap<SongMap>& safeMap, const Genre& genre);

void printSongPaths(const threads::SafeMap<SongMap>& safeMap);

void printSummary(const threads::SafeMap<SongMap>& safeMap, const telemetry::Context& telemetryCtx);

} // namespace helpers::cmdline
