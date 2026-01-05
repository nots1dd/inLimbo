#pragma once

#include "InLimbo-Types.hpp"
#include "StackTrace.hpp"
#include "utils/string/Equals.hpp"
#include "utils/string/SmallString.hpp"

// Include cereal support for SmallString (required for cereal ADL serialization)
// NOLINTNEXTLINE(build/include)
#include "utils/string/SmallStringCereal.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <map>
#include <sys/stat.h> // for inode/stat lookup

namespace core
{

enum class VisitResult
{
  Continue,
  Stop
};

using SongTreeVisitor =
  std::function<VisitResult(const Artist&, const Album&, Disc, Track, const Song&)>;

using ArtistVisitor = std::function<VisitResult(const Artist&)>;
using AlbumVisitor  = std::function<VisitResult(const Artist&, const Album&)>;
using DiscVisitor   = std::function<VisitResult(const Artist&, const Album&, Disc)>;

struct SongTreeCallbacks
{
  std::function<void()> onBegin  = {};
  ArtistVisitor         onArtist = {};
  AlbumVisitor          onAlbum  = {};
  DiscVisitor           onDisc   = {};
  SongTreeVisitor       onSong   = {};
  std::function<void(int artists, int albums, int discs, int songs, int uniqueGenres)> onSummary =
    {};
  std::function<void()> onEnd = {};
};

using SongPredicate = std::function<bool(const Artist&, const Album&, Disc, Track, const Song&)>;

namespace song::sort
{

inline auto byTitle(const Title& t) -> SongPredicate
{
  return [t](auto&, auto&, auto, auto, const Song& s) -> bool
  { return utils::string::isEquals(s.metadata.title, t); };
}

inline auto byTrack(int track) -> SongPredicate
{
  return [track](auto&, auto&, auto, int trk, auto&) -> bool { return trk == track; };
}

inline auto byArtist(const Artist& a) -> SongPredicate
{
  return [a](const Artist& artist, auto&, auto, auto, auto&) -> bool
  { return utils::string::isEquals(artist, a); };
}

inline auto byAlbum(const Album& a) -> SongPredicate
{
  return [a](auto, const Album& album, auto, auto, auto&) -> bool
  { return utils::string::isEquals(album, a); };
}

inline auto byArtistDisc(const Artist& a, Disc d) -> SongPredicate
{
  return [a, d](const Artist& artist, auto&, Disc disc, auto, auto&) -> bool
  { return utils::string::isEquals(artist, a) && disc == d; };
}

inline auto byAlbumDisc(const Album& al, Disc d) -> SongPredicate
{
  return [al, d](auto, const Album& album, auto disc, auto, auto&) -> bool
  { return utils::string::isEquals(album, al) && disc == d; };
}

inline auto byDisc(Disc d) -> SongPredicate
{
  return [d](auto&, auto&, Disc disc, auto, auto&) -> bool { return disc == d; };
}

inline auto byArtistAlbum(const Artist& a, const Album& al) -> SongPredicate
{
  return [a, al](const Artist& artist, const Album& album, auto, auto, auto&) -> bool
  { return utils::string::isEquals(artist, a) && utils::string::isEquals(album, al); };
}

inline auto byYear(uint year) -> SongPredicate
{
  return [year](auto&, auto&, auto, auto, const Song& s) -> bool
  { return s.metadata.year == year; };
}

inline auto byGenre(const Genre& g) -> SongPredicate
{
  return [g](auto&, auto&, auto, auto, const Song& s) -> bool
  { return utils::string::isEquals(s.metadata.genre, g); };
}

inline auto allOf(SongPredicate a, SongPredicate b) -> SongPredicate
{
  return [=](auto&&... args) -> auto { return a(args...) && b(args...); };
}

inline auto anyOf(SongPredicate a, SongPredicate b) -> SongPredicate
{
  return [=](auto&&... args) -> auto { return a(args...) || b(args...); };
}

inline auto notOf(SongPredicate p) -> SongPredicate
{
  return [=](auto&&... args) -> auto { return !p(args...); };
}

} // namespace song::sort

// ============================================================
// SongTree Declaration (NOT THREAD SAFE)
// ============================================================
//
// Note that this structure is used ONLY for serialization and deserialization.
// The in-memory representation used during runtime is a threads::SafeMap<SongMap>.
//
// This is majorly also used for cmdline querying and printing the song library.

class SongTreeRange;

class SongTree
{
private:
  SongMap m_songMap;
  Path    m_musicPath;

public:
  // Core methods
  void               addSong(const Song& song);
  [[nodiscard]] auto range(SongPredicate pred) const -> SongTreeRange;
  void               setMusicPath(const Path& path) { m_musicPath = path; }

  void clear()
  {
    RECORD_FUNC_TO_BACKTRACE("SongTree::clear");
    m_songMap.clear();
    m_musicPath.clear();
  }

  // Query methods
  [[nodiscard]] auto returnSongMap() const { return m_songMap; }
  // note that this replaces the entire song map and newMap is no longer valid after this call
  [[nodiscard]] auto newSongMap(const SongMap& newMap)
  {
    RECORD_FUNC_TO_BACKTRACE("SongTree::replaceSongMap");
    m_songMap = std::move(newMap);
  }
  [[nodiscard]] auto returnMusicPath() const { return m_musicPath; }

  // Persistence
  template <class Archive> void serialize(Archive& ar) { ar(m_songMap, m_musicPath); }

  void saveToFile(const utils::string::SmallString& filename) const;
  void loadFromFile(const utils::string::SmallString& filename);
};

} // namespace core
