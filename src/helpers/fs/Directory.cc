#include "helpers/fs/Directory.hpp"
#include "Logger.hpp"
#include "utils/DirectoryWalker.hpp"

namespace helpers::fs
{

void dirWalkProcessAll(const Directory&                                 directory,
                       utils::RedBlackTree<ino_t, utils::rbt::NilNode>& rbt,
                       core::InodeFileMapper& mapper, core::TagLibParser& parser,
                       core::SongTree& songTree)
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

      // 1. Track inode
      rbt.insert(st.st_ino, utils::rbt::NilNode{});

      // 2. Map inode -> path
      mapper.addMapping(st.st_ino, path.c_str(), true);

      // 3. Parse metadata
      Metadata md;
      if (!parser.parseFile(path, md))
      {
        LOG_WARN("Unable to parse: {}", path);
        return;
      }

      // 4. Add song directly
      songTree.addSong(Song{st.st_ino, md});
    });
}

} // namespace helpers::fs
