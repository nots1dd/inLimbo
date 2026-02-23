#include "helpers/fs/Directory.hpp"
#include "Logger.hpp"
#include "utils/DirectoryWalker.hpp"

namespace helpers::fs
{

// does a singular pass over the given directory, parses any plausible audio files (mp3, flac, ogg,
// ...) then creates a Song obj (with inode and song file metadata) and stores in
// songLibrarySnapshot object.
//
// Note that we arent immediately populating the SongMap as we are still yet to serialize to the
// cache file via cereal.
void dirWalkProcessAll(const Directory& directory, taglib::Parser& tagParser,
                       core::SongLibrarySnapshot& songLibrarySnapshot)
{
  RECORD_FUNC_TO_BACKTRACE("helpers::fs::dirWalkProcessAll");

  utils::DirectoryWalker walker(directory);

  walker.walk(
    [&](const char* name, const struct stat& st, [[maybe_unused]] int parentFd) -> void
    {
      if (!S_ISREG(st.st_mode))
        return;

      Path path;
      path += directory;
      path += '/';
      path += name;

      if (path.empty())
      {
        LOG_CRITICAL("Empty path generated for inode {}", st.st_ino);
        return;
      }

      Metadata md;
      if (!tagParser.parseFile(path, md))
      {
        LOG_WARN("Unable to parse metadata for path: '{}'", path);
        return;
      }

      Song song{st.st_ino, md};
      songLibrarySnapshot.addSong(song);
    });
}

} // namespace helpers::fs
