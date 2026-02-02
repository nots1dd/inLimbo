#pragma once

#include "Event.hpp"
#include "Stats.hpp"

#include "utils/ankerl/Cereal.hpp"
#include <string>
#include <vector>

namespace telemetry
{

class Store
{
public:
  void onEvent(const Event& ev);

  // ---- queries ----
  [[nodiscard]] auto song(SongID id) const -> const Stats*;
  [[nodiscard]] auto artist(ArtistID id) const -> const Stats*;

  [[nodiscard]] auto songs() const -> const ankerl::unordered_dense::map<SongID, Stats>&;
  [[nodiscard]] auto artists() const -> const ankerl::unordered_dense::map<ArtistID, Stats>&;

  // Disk API
  [[nodiscard]] auto save(const std::string& path) const -> bool;
  auto               load(const std::string& path) -> bool;

  template <class Archive> void save(Archive& ar) const;

  template <class Archive> void load(Archive& ar);

private:
  std::vector<Event> events;

  ankerl::unordered_dense::map<SongID, Stats>   songStats;
  ankerl::unordered_dense::map<ArtistID, Stats> artistStats;
  ankerl::unordered_dense::map<AlbumID, Stats>  albumStats;
  ankerl::unordered_dense::map<GenreID, Stats>  genreStats;

  static auto find(const auto& map, auto id) -> const Stats*;

  void apply(const Event& ev);
};

} // namespace telemetry
