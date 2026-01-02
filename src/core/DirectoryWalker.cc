#include "core/DirectoryWalker.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

namespace core
{

auto DirectoryWalker::walk(const EntryCallback& cb) -> bool
{
  RECORD_FUNC_TO_BACKTRACE("DirectoryWalker::walk");

  int fd = open(m_root.c_str(), O_RDONLY | O_DIRECTORY);
  if (fd < 0)
  {
    const std::string errMsg = "Failed to open directory: '" + m_root + "'";
    // We cannot continue if dirwalk fails
    throw std::runtime_error(errMsg);
  }

  walkFd(fd, cb);
  close(fd);
  return true;
}

void DirectoryWalker::walkFd(int dirFd, const EntryCallback& cb)
{
  DIR* dir = fdopendir(dirFd);
  if (!dir)
  {
    LOG_ERROR("fdopendir failed");
    close(dirFd);
    return;
  }

  struct dirent* ent;
  while ((ent = readdir(dir)) != nullptr)
  {
    if (ent->d_name[0] == '.' &&
        (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
      continue;

    struct stat st{};
    if (fstatat(dirFd, ent->d_name, &st, AT_SYMLINK_NOFOLLOW) != 0)
      continue;

    if (S_ISLNK(st.st_mode))
    {

      // default symlink policy is set to ignore, metadata can be retrived with this and
      // will not follow the symlink.
      //
      // If ignore policy doesnt work, try to use report or follow but unlikely that this will help.
      //
      // Report: records symlink paths but doesnt recurse thru it and the caller (cb) will decide
      // what to do. this can be used to showcase ALL the symlinks and do something about it.
      //
      // Follow: (NOT RECOMMENDED!) This follows the symlink and recursively walks thru it if
      // directory else callback is invoked. there *shouldnt* be a need for this as this can cause
      // infinite recursion and a whole lotta problems.
      if (m_symlinkPolicy == SymlinkPolicy::Ignore)
        continue;

      if (m_symlinkPolicy == SymlinkPolicy::Report)
      {
        cb(ent->d_name, st, dirFd);
        continue;
      }

      if (m_symlinkPolicy == SymlinkPolicy::Follow)
      {
        struct stat target{};
        if (fstatat(dirFd, ent->d_name, &target, 0) != 0)
          continue;

        if (S_ISDIR(target.st_mode))
        {
          int childFd = openat(dirFd, ent->d_name, O_RDONLY | O_DIRECTORY);
          if (childFd >= 0)
            walkFd(childFd, cb);
        }
        else
        {
          cb(ent->d_name, target, dirFd);
        }

        continue;
      }
    }

    cb(ent->d_name, st, dirFd);
  }

  closedir(dir); // closes dirFd
}

} // namespace core
