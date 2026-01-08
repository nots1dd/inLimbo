#pragma once

#include "core/SongTree.hpp"
#include <optional>

namespace helpers::cmdline
{

// ------------------------------------------------------------
// Print all frontend plugins built
// ------------------------------------------------------------
void printFrontendPlugins();

// ------------------------------------------------------------
// Print all artists
// ------------------------------------------------------------
void printArtists(const core::SongTree& tree);

// ------------------------------------------------------------
// Print song info
// ------------------------------------------------------------
void printSongInfo(const core::SongTree& tree, const std::optional<std::string>& songName);

// ------------------------------------------------------------
// Print song lyrics
// ------------------------------------------------------------
void printSongLyrics(const core::SongTree& tree, const Title& songTitle);

// ------------------------------------------------------------
// Print albums (optionally filtered by artist)
// ------------------------------------------------------------
void printAlbums(const core::SongTree& tree, const std::optional<Artist>& artist = std::nullopt);

// ------------------------------------------------------------
// Print genres
// ------------------------------------------------------------
void printGenres(const core::SongTree& tree);

// ------------------------------------------------------------
// Print songs (optionally filtered)
// ------------------------------------------------------------
void printSongs(const core::SongTree& tree, query::song::SongPredicate pred = {});

void printSongsByArtist(const core::SongTree& tree, const Artist& artist);
void printSongsByAlbum(const core::SongTree& tree, const Album& album);
void printSongsByGenre(const core::SongTree& tree, const Genre& genre);

// ------------------------------------------------------------
// Print song paths (title, artist, absolute path)
// ------------------------------------------------------------
void printSongPaths(const core::SongTree& tree, query::song::SongPredicate pred = {});

// ------------------------------------------------------------
// Print library summary
// ------------------------------------------------------------
void printSummary(const core::SongTree& tree);

// ------------------------------------------------------------
// Print songs by artist + album
// ------------------------------------------------------------
void printAlbum(const core::SongTree& tree, const Artist& artist, const Album& album);

} // namespace helpers::cmdline
