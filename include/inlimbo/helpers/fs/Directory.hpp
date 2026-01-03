#pragma once

#include "StackTrace.hpp"
#include "core/DirectoryWalker.hpp"
#include "core/InodeMapper.hpp"
#include "utils/RBTree.hpp"
#include <cstring>
#include <string>

namespace helpers::fs
{

inline void dirWalkAndUpdateRBT(const std::string&                               directoryPath,
                                utils::RedBlackTree<ino_t, utils::rbt::NilNode>& rbt,
                                core::InodeFileMapper&                           mapper)
{
  RECORD_FUNC_TO_BACKTRACE("ProcessDirectory");

  core::DirectoryWalker dirwalker(directoryPath);

  dirwalker.walk(
    [&](const char* name, const struct stat& st, int parentFd) -> void
    {
      if (!S_ISREG(st.st_mode))
        return;

      // Build path lazily ONLY if needed
      std::string fullPath;
      fullPath.reserve(directoryPath.size() + 1 + std::strlen(name));
      fullPath.append(directoryPath).append("/").append(name);

      rbt.insert(st.st_ino, utils::rbt::NilNode{});
      mapper.addMapping(st.st_ino, fullPath, true);
    });
}

} // namespace helpers::fs
