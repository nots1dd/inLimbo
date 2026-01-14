#pragma once

#include "InLimbo-Types.hpp"
#include <string>

namespace config
{

class Watcher
{
public:
  explicit Watcher(std::string filePath);
  ~Watcher();

  Watcher(const Watcher&)                    = delete;
  auto operator=(const Watcher&) -> Watcher& = delete;

  auto pollChanged() -> bool;

private:
  void setupWatch();

private:
  PathStr m_filePath;
  PathStr m_dirPath;
  PathStr m_fileName;

  int  m_fd{-1};
  int  m_wd{-1};
  bool m_dirty{false};
};

} // namespace config
