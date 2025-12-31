#pragma once

#include <string>
#include "StackTrace.hpp"
#include "core/DirectoryWalker.hpp"
#include "core/InodeMapper.hpp"
#include "core/RBTree.hpp"
#include "core/SongTree.hpp"

namespace helpers
{

inline void processDirectory(const std::string& directoryPath,
                             dirsort::RedBlackTree<ino_t, core::Song>& rbt,
                             core::InodeFileMapper& mapper)
{
    RECORD_FUNC_TO_BACKTRACE("ProcessDirectory");

    core::DirectoryWalker dirwalker(directoryPath);

    dirwalker.walk([&](const char* name,
                    const struct stat& st,
                    int parentFd) -> void {

        if (!S_ISREG(st.st_mode))
            return;

        // Build path lazily ONLY if needed
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s",
                 directoryPath.c_str(), name);

        rbt.insert(st.st_ino);
        mapper.addMapping(st.st_ino, path, true);
    });
}

}
