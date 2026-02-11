#pragma once

#include "InLimbo-Types.hpp"

namespace inlimbo
{

struct Args
{
  // General
  Title       song;
  std::string frontend;
  bool        listFrontend = false;
  float       volume       = 75.0f;
  bool        fuzzySearch  = false;

  // Edit
  Title  editTitle;
  Artist editArtist;
  Album  editAlbum;
  Genre  editGenre;
  Lyrics editLyrics;
  // to fetch lyrics from external sources and embed into metadata
  bool fetchLyricsMode = false;

  // Modify
  bool rebuildLibrary = false;

  // Query
  bool printArtists = false;
  bool printAlbums  = false;
  bool printGenre   = false;
  bool printSummary = false;
  bool songsPaths   = false;

  Title printSong;
  Title printLyrics;

  Title songsArtist;
  Title songsAlbum;
  Title songsGenre;
};

} // namespace inlimbo
