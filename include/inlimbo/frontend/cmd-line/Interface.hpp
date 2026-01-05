#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>

#include "audio/Service.hpp"
#include "core/SongTree.hpp"
#include "thread/Map.hpp"

namespace frontend::cmdline
{

class Interface
{
public:
  explicit Interface(threads::SafeMap<SongMap>& songMap);

  void run(audio::Service& audio);

private:
  threads::SafeMap<SongMap> m_songMapTS;
  std::atomic<bool>         m_isRunning{false};
  std::atomic<double>       m_pendingSeek{0.0};

  termios m_termOrig{};

  std::mutex              m_metaMutex;
  std::optional<Metadata> m_currentMeta;

  struct TermSize
  {
    int rows = 0;
    int cols = 0;
  };

  static auto getTerminalSize() -> TermSize;

  void enableRawMode();
  void disableRawMode();

  void statusLoop(audio::Service& audio);
  void inputLoop(audio::Service& audio);
  void seekLoop(audio::Service& audio);

  void draw(audio::Service& audio);
  void drawTooSmall(const TermSize& ts);

  auto handleKey(audio::Service& audio, char c) -> bool;

  static void showMetadata(const Metadata& m);
};

} // namespace frontend::cmdline
