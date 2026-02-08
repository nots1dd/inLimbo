#pragma once

#include <atomic>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>

#include "audio/Service.hpp"
#include "config/Watcher.hpp"
#include "frontend/cmdline/Structs.hpp"
#include "mpris/Service.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"
#include "utils/ASCII.hpp"
#include "utils/ClassRulesMacros.hpp"
#include "utils/Snapshot.hpp"
#include "utils/fs/Paths.hpp"

static constexpr cstr FRONTEND_NAME = "cmdline";

namespace frontend::cmdline
{

enum class UiMode : ui8
{
  Normal,
  SearchTitle,
  SearchArtist
};

class Interface
{
public:
  // NOTE: pointers because this is created via C ABI
  explicit Interface(threads::SafeMap<SongMap>* songMap, telemetry::Context* telemetry,
                     mpris::Service* mprisService)
      : m_cfgWatcher(utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_CONFIG_FILE_NAME)),
        m_songMapTS(songMap), m_telemetryCtx(telemetry), m_mprisService(mprisService)
  {
  }

  IMMUTABLE(Interface);

  [[nodiscard]] auto ready() -> bool;
  void               run(audio::Service& audio);

private:
  UiMode                     m_mode{UiMode::Normal};
  std::string                m_searchBuf;
  config::Watcher            m_cfgWatcher;
  threads::SafeMap<SongMap>* m_songMapTS{nullptr};
  telemetry::Context*        m_telemetryCtx{nullptr};
  mpris::Service*            m_mprisService{nullptr};
  mutable std::mutex         m_renderMutex;

  // config
  utils::Snapshot<CmdlineConfig> m_cfg{};

  std::atomic<bool>   m_isRunning{false};
  std::atomic<double> m_pendingSeek{0.0};
  std::atomic<ui8>    m_lastAutoNextTid{0};
  std::atomic<bool>   m_autoNextInProgress{false};
  std::optional<i64>  m_lastPlayTick;

  // telemetry
  std::optional<telemetry::Event> m_currentPlay; // active play session

  termios m_termOrig{};

  struct TermSize
  {
    int rows = 0;
    int cols = 0;
  };

  static auto getTerminalSize() -> TermSize;

  void loadConfig();
  void loadMiscConfig(MiscConfig& miscCfg);

  void enableRawMode();
  void disableRawMode();

  void statusLoop(audio::Service& audio);
  void inputLoop(audio::Service& audio);
  void seekLoop(audio::Service& audio);

  template <typename OnSubmit>
  auto handleSearchCommon(char c, OnSubmit&& onSubmit) -> bool;
  auto handleSearchTitleMode(audio::Service& audio, char c) -> bool;
  auto handleSearchArtistMode(audio::Service& audio, char c) -> bool;

  void draw(audio::Service& audio);
  void drawBottomPrompt(const TermSize& ts, const UiColors& colors, std::string_view label,
                        std::string_view text);
  void drawTooSmall(const TermSize& ts);

  auto handleKey(audio::Service& audio, char c) -> bool;

  static void showMetadata(const Metadata& m);
};

template <typename OnSubmit>
auto Interface::handleSearchCommon(config::keybinds::KeyChar character, OnSubmit&& onSubmit) -> bool
{
  // ESC exits search
  if (character == utils::ascii::ESC)
  {
    m_mode = UiMode::Normal;
    return true;
  }

  // ENTER submits
  if (utils::ascii::isEnter(character))
    return onSubmit();

  // Backspace
  if (character == utils::ascii::DEL || character == utils::ascii::BS)
  {
    if (!m_searchBuf.empty())
      m_searchBuf.pop_back();
    return true;
  }

  // Printable ASCII
  if (utils::ascii::isPrintable(character))
  {
    if (m_searchBuf.size() < 128)
      m_searchBuf.push_back(character);
    return true;
  }

  return true;
}

} // namespace frontend::cmdline
