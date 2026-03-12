#pragma once

#include "Event.hpp"
#include "Logger.hpp"
#include "Stats.hpp"

#include "utils/ankerl/Cereal.hpp"
#include <string>
#include <vector>

namespace telemetry
{

/// a generic map that corresponds to T -> key && Stats -> value
///
/// ex: StatsMap<SongID> -> for each song id we have a corresponding stat struct
/// describing about the song (like no of plays, total playtime, last played, etc.)
template <typename T>
using StatsMap = ankerl::unordered_dense::map<T, Stats>;

class Store
{
public:
  explicit Store(double minPlaySec) : minPlaySec(minPlaySec)
  {
    LOG_DEBUG("Telemetry store initializing. Min Playback Event capture time set to '{}'s",
              minPlaySec);
  }
  void onEvent(const Event& ev);

  // ---- queries ----
  [[nodiscard]] auto song(SongID id) const -> const Stats*;
  [[nodiscard]] auto artist(ArtistID id) const -> const Stats*;
  [[nodiscard]] auto album(AlbumID id) const -> const Stats*;
  [[nodiscard]] auto genre(GenreID id) const -> const Stats*;

  [[nodiscard]] auto songs() const -> const StatsMap<SongID>&;
  [[nodiscard]] auto artists() const -> const StatsMap<ArtistID>&;
  [[nodiscard]] auto albums() const -> const StatsMap<AlbumID>&;
  [[nodiscard]] auto genres() const -> const StatsMap<GenreID>&;

  // Disk API
  [[nodiscard]] auto save(const std::string& path) const -> bool;
  auto               load(const std::string& path) -> bool;

  template <class Archive>
  void save(Archive& ar) const;
  template <class Archive>
  void load(Archive& ar);

private:
  std::vector<Event> events;
  double             minPlaySec = 30.0;

  StatsMap<SongID>   songStats;
  StatsMap<ArtistID> artistStats;
  StatsMap<AlbumID>  albumStats;
  StatsMap<GenreID>  genreStats;

  static auto find(const auto& map, auto id) -> const Stats*;

  void apply(const Event& ev);
};

} // namespace telemetry
