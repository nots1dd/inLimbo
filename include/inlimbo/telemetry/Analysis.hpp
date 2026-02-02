#pragma once
#include "telemetry/Store.hpp"

namespace telemetry::analysis
{

// Totals
auto totalListenTime(const Store& s) -> double;

// Favorites (by time)
auto favoriteArtist(const Store& s) -> ArtistID;
auto favoriteAlbum(const Store& s) -> AlbumID;
auto favoriteGenre(const Store& s) -> GenreID;

// Favorites (by plays)
auto mostReplayedSong(const Store& s) -> SongID;

// Recent activity
auto hottestArtist(const Store& s, Timestamp now) -> ArtistID;
// auto  hottestGenre(const Store& s, Timestamp now) -> GenreID;
auto hottestSong(const Store& s, Timestamp now) -> SongID;

// Engagement
auto averageSongListenTime(const Store& s) -> double;

} // namespace telemetry::analysis
