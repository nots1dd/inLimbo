#pragma once

#include <functional>
#include <string>
#include <utility>
#include <sys/stat.h>

namespace core {

class DirectoryWalker {
public:
    enum class SymlinkPolicy {
        Ignore,
        Report,
        Follow
    };

    using EntryCallback =
        std::function<void(const char* name,
                           const struct stat& st,
                           int parentFd)>;

    explicit DirectoryWalker(std::string  root,
                             SymlinkPolicy policy = SymlinkPolicy::Ignore)
        : m_root(std::move(root)),
          m_symlinkPolicy(policy) {}

    auto walk(const EntryCallback& cb) -> bool;

private:
    std::string   m_root;
    SymlinkPolicy m_symlinkPolicy;

    void walkFd(int dirFd, const EntryCallback& cb);
};

} // namespace core
