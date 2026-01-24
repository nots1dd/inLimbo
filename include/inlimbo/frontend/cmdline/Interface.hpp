#pragma once

#include <atomic>
#include <mutex>
#include <optional>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>

#include "audio/Service.hpp"
#include "config/Watcher.hpp"
#include "frontend/cmdline/Structs.hpp"
#include "mpris/Service.hpp"
#include "thread/Map.hpp"
#include "utils/PathResolve.hpp"
#include "utils/Snapshot.hpp"

static constexpr cstr FRONTEND_NAME = "cmdline";

namespace frontend::cmdline
{

enum class UiMode : ui8
{
  Normal,
  SearchSong,
};

class Interface
{
public:
  // NOTE: pointers because this is created via C ABI
  explicit Interface(threads::SafeMap<SongMap>* songMap, mpris::Service* mprisService)
      : m_cfgWatcher(utils::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME)),
        m_songMapTS(songMap), m_mprisService(mprisService)
  {
  }

  Interface(const Interface&)                    = delete;
  auto operator=(const Interface&) -> Interface& = delete;

  void run(audio::Service& audio);

private:
  UiMode                     m_mode{UiMode::Normal};
  std::string                m_searchBuf;
  config::Watcher            m_cfgWatcher;
  threads::SafeMap<SongMap>* m_songMapTS{nullptr};
  mpris::Service*            m_mprisService{nullptr};

  // config
  utils::Snapshot<CmdlineConfig> m_cfg{};

  std::atomic<bool>   m_isRunning{false};
  std::atomic<double> m_pendingSeek{0.0};

  termios m_termOrig{};

  std::mutex              m_metaMutex;
  std::optional<Metadata> m_currentMeta;

  struct TermSize
  {
    int rows = 0;
    int cols = 0;
  };

  static auto getTerminalSize() -> TermSize;

  void loadConfig();

  void enableRawMode();
  void disableRawMode();

  void statusLoop(audio::Service& audio);
  void inputLoop(audio::Service& audio);
  void seekLoop(audio::Service& audio);

  auto handleSearchSongMode(audio::Service& audio, char c) -> bool;

  void draw(audio::Service& audio);
  void drawBottomPrompt(const TermSize& ts, const UiColors& colors, std::string_view label,
                        std::string_view text);
  void drawTooSmall(const TermSize& ts);

  auto handleKey(audio::Service& audio, char c) -> bool;

  static void showMetadata(const Metadata& m);
};

} // namespace frontend::cmdline
