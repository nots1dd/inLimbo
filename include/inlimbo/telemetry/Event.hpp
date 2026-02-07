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
  Timestamp timestamp; // unix time

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(type, song, artist, album, genre, seconds, timestamp);
  }
};

} // namespace telemetry
