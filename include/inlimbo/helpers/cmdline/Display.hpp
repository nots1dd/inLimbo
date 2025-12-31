#pragma once

#include "core/SongTreeIter.hpp"
#include <iostream>
#include <set>
#include <map>

using namespace core;

namespace helpers::cmdline
{

// ------------------------------------------------------------
// Print all artists
// ------------------------------------------------------------
inline void printArtists(const SongTree& tree)
{
    std::set<Artist> artists;

    for (const Song& s : tree.range({})) {
        artists.insert(s.metadata.artist);
    }

    std::cout << "\nArtists:\n";
    std::cout << "────────────────────────────\n";
    for (const auto& a : artists)
        std::cout << "• " << a << "\n";
}

// ------------------------------------------------------------
// Print albums (optionally filtered by artist)
// ------------------------------------------------------------
inline void printAlbums(
    const SongTree& tree,
    const std::optional<Artist>& artist = std::nullopt)
{
    std::map<Artist, std::set<Album>> albums;

    SongPredicate pred = artist
        ? song::sort::byArtist(*artist)
        : SongPredicate{};

    for (const Song& s : tree.range(pred)) {
        albums[s.metadata.artist].insert(s.metadata.album);
    }

    std::cout << "\nAlbums:\n";
    std::cout << "────────────────────────────\n";

    for (const auto& [a, als] : albums) {
        std::cout << a << "\n";
        for (const auto& al : als)
            std::cout << "  └─ " << al << "\n";
    }
}

// ------------------------------------------------------------
// Print genres
// ------------------------------------------------------------
inline void printGenres(const SongTree& tree)
{
    std::set<Genre> genres;

    for (const Song& s : tree.range({})) {
        genres.insert(s.metadata.genre);
    }

    std::cout << "\nGenres:\n";
    std::cout << "────────────────────────────\n";
    for (const auto& g : genres)
        std::cout << "• " << g << "\n";
}

// ------------------------------------------------------------
// Print songs (optionally filtered)
// ------------------------------------------------------------
inline void printSongs(
    const SongTree& tree,
    SongPredicate pred = {})
{
    std::cout << "\nSongs:\n";
    std::cout << "────────────────────────────\n";

    for (const Song& s : tree.range(pred)) {
        std::cout
            << "• " << s.metadata.title
            << " — " << s.metadata.artist
            << " [" << s.metadata.album << "]\n";
    }
}

// ------------------------------------------------------------
// Print library summary
// ------------------------------------------------------------
inline void printSummary(const SongTree& tree)
{
    std::set<Artist> artists;
    std::set<Album>  albums;
    std::set<Genre>  genres;
    size_t songs = 0;

    for (const Song& s : tree.range({})) {
        ++songs;
        artists.insert(s.metadata.artist);
        albums.insert(s.metadata.album);
        genres.insert(s.metadata.genre);
    }

    std::cout << "\nLibrary Summary\n";
    std::cout << "────────────────────────────\n";
    std::cout << "Artists : " << artists.size() << "\n";
    std::cout << "Albums  : " << albums.size() << "\n";
    std::cout << "Songs   : " << songs << "\n";
    std::cout << "Genres  : " << genres.size() << "\n";
}

// ------------------------------------------------------------
// Print songs by artist + album
// ------------------------------------------------------------
inline void printAlbum(
    const SongTree& tree,
    const Artist& artist,
    const Album& album)
{
    auto pred = song::sort::byAlbum(artist, album);

    std::cout << "\n" << artist << " — " << album << "\n";
    std::cout << "────────────────────────────\n";

    for (const Song& s : tree.range(pred)) {
        std::cout
            << std::setw(2) << s.metadata.track
            << ". " << s.metadata.title << "\n";
    }
}

} // namespace core::cli
