#include "helpers/fs/Directory.hpp"
#include "Logger.hpp"
#include "utils/DirectoryWalker.hpp"

namespace helpers::fs
{

// does a singular pass over the given directory, parses any plausible audio files (mp3, flac, wav,
// ...) then creates a Song obj (with inode and song file metadata) and stores in songTree
// datastructure.
//
// Note that we arent immediately populating the SongMap as we are still yet to serialize to the
// cache file via cereal.
void dirWalkProcessAll(const Directory& directory, taglib::Parser& parser, core::SongTree& songTree)
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
      if (!parser.parseFile(path, md))
      {
        LOG_WARN("Unable to parse metadata for path: '{}'", path);
        return;
      }

      Song song{st.st_ino, md};
      songTree.addSong(song);
    });
}

} // namespace helpers::fs
