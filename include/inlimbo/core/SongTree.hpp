#pragma once

#include "InLimbo-Types.hpp"
#include "StackTrace.hpp"
#include "query/Predicates.hpp"
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
  [[nodiscard]] auto range(query::song::SongPredicate pred) const -> SongTreeRange;
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
