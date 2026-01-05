#pragma once

#include "utils/string/SmallString.hpp"
#include <functional>
#include <string>
#include <sys/stat.h>
#include <utility>

namespace utils
{

class DirectoryWalker
{
public:
  enum class SymlinkPolicy
  {
    Ignore,
    Report,
    Follow
  };

  using EntryCallback = std::function<void(const char* name, const struct stat& st, int parentFd)>;

  explicit DirectoryWalker(utils::string::SmallString root,
                           SymlinkPolicy              policy = SymlinkPolicy::Ignore)
      : m_root(std::move(root)), m_symlinkPolicy(policy)
  {
  }

  auto walk(const EntryCallback& cb) -> bool;

private:
  std::string   m_root;
  SymlinkPolicy m_symlinkPolicy;

  void walkFd(int dirFd, const EntryCallback& cb);
};

} // namespace utils
