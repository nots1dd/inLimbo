#pragma once

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <stdexcept>
#include <string>

namespace utils::unix
{

struct LockFileError : public std::runtime_error
{
  explicit LockFileError(const std::string& msg) : std::runtime_error(msg) {}
};

struct LockFileOpenError : public LockFileError
{
  explicit LockFileOpenError(const std::string& path)
      : LockFileError("Failed to open lockfile: " + path)
  {
  }
};

struct LockFileAlreadyLocked : public LockFileError
{
  LockFileAlreadyLocked() : LockFileError("Another instance is already running!") {}
};

class LockFile
{
public:
  explicit LockFile(const std::string& path) : m_path(path)
  {
    m_fd = ::open(path.c_str(), O_CREAT | O_RDWR, 0644);
    if (m_fd < 0)
      throw LockFileOpenError(path);

    if (::flock(m_fd, LOCK_EX | LOCK_NB) != 0)
    {
      ::close(m_fd);
      m_fd = -1;
      throw LockFileAlreadyLocked();
    }
  }

  LockFile(const LockFile&)                    = delete;
  auto operator=(const LockFile&) -> LockFile& = delete;

  LockFile(LockFile&& other) noexcept : m_fd(other.m_fd), m_path(std::move(other.m_path))
  {
    other.m_fd = -1;
  }

  auto operator=(LockFile&& other) noexcept -> LockFile&
  {
    if (this != &other)
    {
      release();
      m_fd       = other.m_fd;
      m_path     = std::move(other.m_path);
      other.m_fd = -1;
    }
    return *this;
  }

  ~LockFile() { release(); }

private:
  int         m_fd = -1;
  std::string m_path;

  void release() noexcept
  {
    if (m_fd >= 0)
    {
      ::flock(m_fd, LOCK_UN);
      ::close(m_fd);
      m_fd = -1;
    }
  }
};

} // namespace utils::unix
