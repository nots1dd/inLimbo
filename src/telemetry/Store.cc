#include "telemetry/Store.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include <fstream>

namespace telemetry
{

void Store::onEvent(const Event& ev)
{
  events.push_back(ev);
  apply(ev);
}

auto Store::song(SongID id) const -> const Stats* { return find(songStats, id); }

auto Store::artist(ArtistID id) const -> const Stats* { return find(artistStats, id); }

auto Store::songs() const -> const ankerl::unordered_dense::map<SongID, Stats>&
{
  return songStats;
}

auto Store::artists() const -> const ankerl::unordered_dense::map<ArtistID, Stats>&
{
  return artistStats;
}

auto Store::save(const std::string& path) const -> bool
{
  std::ofstream os(path, std::ios::binary);
  if (!os)
    return false;

  cereal::BinaryOutputArchive ar(os);
  ar(*this);
  return true;
}

auto Store::load(const std::string& path) -> bool
{
  std::ifstream is(path, std::ios::binary);
  if (!is)
    return false;

  cereal::BinaryInputArchive ar(is);
  ar(*this);
  return true;
}

template <class Archive> void Store::save(Archive& ar) const
{
  ar(events, songStats, artistStats, albumStats, genreStats);
}

template <class Archive> void Store::load(Archive& ar)
{
  ar(events, songStats, artistStats, albumStats, genreStats);
}

// ------------------------------------------------------------
// Internals
// ------------------------------------------------------------

auto Store::find(const auto& map, auto id) -> const Stats*
{
  auto it = map.find(id);
  return it == map.end() ? nullptr : &it->second;
}

void Store::apply(const Event& ev)
{
  if (ev.type != EventType::PlayEnd)
    return;

  songStats[ev.song].update(ev.seconds, ev.timestamp);
  artistStats[ev.artist].update(ev.seconds, ev.timestamp);
  albumStats[ev.album].update(ev.seconds, ev.timestamp);
  genreStats[ev.genre].update(ev.seconds, ev.timestamp);
}

} // namespace telemetry
