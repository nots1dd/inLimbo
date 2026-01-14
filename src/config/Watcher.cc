#include "config/Watcher.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include <stdexcept>

namespace
{

static inline auto splitPath(const PathStr& full, DirectoryStr& dirOut, PathStr& fileOut) -> void
{
  const auto pos = full.find_last_of('/');
  if (pos == std::string::npos)
  {
    dirOut  = ".";
    fileOut = full;
    return;
  }

  dirOut  = full.substr(0, pos);
  fileOut = full.substr(pos + 1);
  if (dirOut.empty())
    dirOut = "/";
}

} // namespace

namespace config
{

Watcher::Watcher(std::string filePath) : m_filePath(std::move(filePath))
{
  splitPath(m_filePath, m_dirPath, m_fileName);

  m_fd = inotify_init1(IN_NONBLOCK);
  if (m_fd < 0)
    throw std::runtime_error("inotify_init1 failed");

  setupWatch();
}

void Watcher::setupWatch()
{
  if (m_wd >= 0)
  {
    inotify_rm_watch(m_fd, m_wd);
    m_wd = -1;
  }

  // watch for create, modify, close_write, moved_to, delete in the directory
  m_wd = inotify_add_watch(m_fd, m_dirPath.c_str(),
                           IN_CREATE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE);

  if (m_wd < 0)
  {
    ::close(m_fd);
    m_fd = -1;
    throw std::runtime_error("inotify_add_watch failed");
  }
}

Watcher::~Watcher()
{
  if (m_wd >= 0 && m_fd >= 0)
    inotify_rm_watch(m_fd, m_wd);

  if (m_fd >= 0)
    ::close(m_fd);
}

auto Watcher::pollChanged() -> bool
{
  if (m_fd < 0)
    return false;

  char buf[4096];

  while (true)
  {
    const ssize_t n = ::read(m_fd, buf, sizeof(buf));
    if (n <= 0)
      break;

    ssize_t i = 0;
    while (i < n)
    {
      const auto* ev = reinterpret_cast<const inotify_event*>(buf + i);

      // react if it's the config file
      if (ev->len > 0)
      {
        if (m_fileName == ev->name)
        {
          if (ev->mask & (IN_CLOSE_WRITE | IN_MOVED_TO))
            m_dirty = true;
        }
      }

      i += sizeof(inotify_event) + ev->len;
    }
  }

  if (m_dirty)
  {
    m_dirty = false;
    return true;
  }

  return false;
}

} // namespace config
