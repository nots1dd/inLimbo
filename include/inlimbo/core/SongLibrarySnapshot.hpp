#pragma once

#include "InLimbo-Types.hpp"
#include "StackTrace.hpp"
#include "utils/string/SmallString.hpp"

// NOLINTNEXTLINE(build/include)
#include "utils/string/SmallStringCereal.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <sys/stat.h> // for inode/stat lookup

namespace core
{

// ============================================================
// SongLibrarySnapshot Declaration (NOT THREAD SAFE)
// ============================================================
//
// Note that this structure is used ONLY for serialization and deserialization.
// The in-memory representation used during runtime is a threads::SafeMap<SongMap>.
//

class SongLibrarySnapshot
{
private:
  SongMap m_songMap;
  Path    m_musicPath;

public:
  // Core methods
  void addSong(const Song& song);
  void setMusicPath(const Path& path) { m_musicPath = path; }

  ~SongLibrarySnapshot() { clear(); }

  void clear()
  {
    RECORD_FUNC_TO_BACKTRACE("SongLibrarySnapshot::clear");
    m_songMap.clear();
    m_musicPath.clear();
  }

  // Query methods
  [[nodiscard]] auto returnSongMap() const -> const SongMap&
  {
    RECORD_FUNC_TO_BACKTRACE("SongLibrarySnapshot::returnSongMap");
    return m_songMap;
  }
  [[nodiscard]] auto moveSongMap() noexcept -> SongMap
  {
    RECORD_FUNC_TO_BACKTRACE("SongLibrarySnapshot::moveSongMap");
    return std::move(m_songMap);
  }
  // note that this replaces the entire song map and newMap is no longer valid after this call
  void newSongMap(const SongMap& newMap)
  {
    RECORD_FUNC_TO_BACKTRACE("SongLibrarySnapshot::replaceSongMap");
    m_songMap = std::move(newMap);
  }
  [[nodiscard]] auto returnMusicPath() const -> const Path { return m_musicPath; }

  // Persistence
  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(m_songMap, m_musicPath);
  }

  void saveToFile(const utils::string::SmallString& filename) const;
  void loadFromFile(const utils::string::SmallString& filename);
};

} // namespace core
