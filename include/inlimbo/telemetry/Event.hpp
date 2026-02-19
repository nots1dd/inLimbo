#pragma once

#include "IDs.hpp"
#include <cereal/types/vector.hpp>

namespace telemetry
{

enum class EventType : ui8
{
  PlayEnd
};

struct Event
{
  EventType type;

  SongID   song;
  ArtistID artist;
  AlbumID  album;
  GenreID  genre;

  double    seconds;   // listened or seek delta
  double    duration;  // duration of the song
  Timestamp timestamp; // unix time

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(type, song, artist, album, genre, seconds, duration, timestamp);
  }
};

} // namespace telemetry
