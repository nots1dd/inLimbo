#include "frontend/cmdline/Interface.hpp"

#include <cmath>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <unistd.h>

#include "Logger.hpp"
#include "config/Colors.hpp"
#include "config/Misc.hpp"
#include "toml/Parser.hpp"

#include "mpris/Service.hpp"
#include "query/SongMap.hpp"
#include "utils/Index.hpp"
#include "utils/timer/Timer.hpp"

namespace colors = config::colors;

namespace frontend::cmdline
{

static constexpr int MIN_TERM_COLS = 80;
static constexpr int MIN_TERM_ROWS = 24;

#define APP_TITLE "inLimbo - CLI Player"

#define UI_BAR_FILL  "█"
#define UI_BAR_EMPTY "░"

static auto autoNextIfFinished(audio::Service& audio, mpris::Service& mpris) -> void
{
  static ui8 lastTid;
  auto       infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
    return;

  const auto& info = *infoOpt;

  if (!info.playing)
    return;

  if (info.lengthSec <= 0.0)
    return;

  if (info.tid == lastTid)
    return;

  constexpr double EPS = 0.10;

  if (info.positionSec + EPS < info.lengthSec)
    return;

  lastTid = info.tid;
  audio.nextTrackGapless();
  mpris.updateMetadata();
  mpris.notify();
}

void Interface::loadMiscConfig(MiscConfig& miscCfg)
{
  config::misc::ConfigLoader loader(FRONTEND_NAME);

  loader.load(
    config::keybinds::Binding<int>{.key = "seek_duration", .target = &miscCfg.seekDuration});
}

void Interface::loadConfig()
{
  try
  {
    tomlparser::Config::load();

    config::colors::ConfigLoader   colorsCfg(FRONTEND_NAME);
    config::keybinds::ConfigLoader keysCfg(FRONTEND_NAME);

    CmdlineConfig next;
    next.kb     = Keybinds::load(FRONTEND_NAME);
    next.colors = UiColors::load(FRONTEND_NAME);

    loadMiscConfig(next.misc);

    m_cfg.set(std::move(next));

    LOG_INFO("Configuration loaded for {}'s keybinds and colors.", FRONTEND_NAME);
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Interface::loadConfig failed: {}", e.what());
  }
}

void Interface::run(audio::Service& audio)
{
  loadConfig();

  // Query is very nice and easy to add. Take this for example:
  //
  // We can add every song after the current one's track no of an
  // album.

  query::songmap::read::forEachSongInAlbum(
    *m_songMapTS, audio.getCurrentMetadata()->artist, audio.getCurrentMetadata()->album,
    [&](const Disc&, const Track&, const ino_t, const Song& song) -> void
    {
      if (song.metadata.track <= audio.getCurrentMetadata()->track)
        return;

      auto h = audio.registerTrack(song);
      audio.addToPlaylist(h);
    });

  const auto audioMetaOpt = audio.getCurrentMetadata();

  if (!audioMetaOpt)
    LOG_ERROR("No current metadata available in audio service!");

  // Or we can just add every song that is not the current one.
  //
  // Note that the songmap is stored in such a way:
  //
  // Artist -> Album -> Disc -> Track -> Song

  // query::songmap::read::forEachSong(
  //   *m_songMapTS,
  //   [&](const Artist&, const Album&, Disc, Track, ino_t, const Song& song) -> void
  //   {
  //     if (song.metadata.title == audioMetaOpt->title)
  //       return;
  //
  //     auto h = audio.registerTrack(song);
  //     audio.addToPlaylist(h);
  //   });

  m_mprisService->updateMetadata();

  enableRawMode();
  m_isRunning.store(true);

  std::thread status([&]() -> void { statusLoop(audio); });
  std::thread input([&]() -> void { inputLoop(audio); });
  std::thread seek([&]() -> void { seekLoop(audio); });

  while (m_isRunning.load())
  {
    draw(audio);
    m_mprisService->poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  m_isRunning.store(false);
  disableRawMode();

  if (input.joinable())
    input.join();
  if (status.joinable())
    status.join();
  if (seek.joinable())
    seek.join();
}

auto Interface::getTerminalSize() -> TermSize
{
  winsize ws{};
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    return {};
  return {.rows = static_cast<int>(ws.ws_row), .cols = static_cast<int>(ws.ws_col)};
}

void Interface::enableRawMode()
{
  tcgetattr(STDIN_FILENO, &m_termOrig);
  termios raw = m_termOrig;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void Interface::disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_termOrig); }

void Interface::statusLoop(audio::Service& audio)
{
  while (m_isRunning.load())
  {

    if (m_cfgWatcher.pollChanged())
    {
      LOG_DEBUG("Configuration file changed, reloading...");
      loadConfig();
    }

    autoNextIfFinished(audio, *m_mprisService);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void Interface::inputLoop(audio::Service& audio)
{
  pollfd pfd{.fd = STDIN_FILENO, .events = POLLIN, .revents = 0};

  while (m_isRunning.load())
  {
    if (poll(&pfd, 1, 25) > 0)
    {
      char c;
      if (read(STDIN_FILENO, &c, 1) > 0)
      {
        if (!handleKey(audio, c))
          break;
      }
    }
  }
  m_isRunning.store(false);
}

void Interface::seekLoop(audio::Service& audio)
{
  while (m_isRunning.load())
  {
    double d = m_pendingSeek.exchange(0.0);
    if (std::abs(d) > 0.01)
    {
      if (d > 0.0)
        audio.seekForward(d);
      else
        audio.seekBackward(-d);
    }
    m_mprisService->notify();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void Interface::drawTooSmall(const TermSize& ts)
{
  auto        cfg    = m_cfg.get();
  const auto& colors = cfg->colors;

  std::cout << colors::Ansi::Clear;

  std::cout << colors::Ansi::Bold << colors.accent << APP_TITLE << colors::Ansi::Reset << "\n\n";

  std::cout << colors::Ansi::Bold << colors.warning << " Terminal too small" << colors::Ansi::Reset
            << "\n\n";

  std::cout << " Current  : " << colors::Ansi::Bold << colors.error << ts.cols << " cols × "
            << ts.rows << " rows" << colors::Ansi::Reset << "\n\n";

  std::cout << colors::Ansi::Dim << colors.fg << " Resize the terminal window."
            << colors::Ansi::Reset << "\n";

  std::cout.flush();
}

void Interface::drawBottomPrompt(const TermSize& ts, const UiColors& colors, std::string_view label,
                                 std::string_view text)
{
  std::cout << colors::Ansi::MoveCursor(ts.rows, 1) << colors::Ansi::ClearLine;

  std::cout << colors::Ansi::Bold << colors.accent << label << colors::Ansi::Reset << colors.fg
            << text << colors::Ansi::Reset;

  std::cout.flush();
}

void Interface::draw(audio::Service& audio)
{
  std::lock_guard<std::mutex> lock(m_renderMutex);

  const auto ts = getTerminalSize();
  if (ts.cols < MIN_TERM_COLS || ts.rows < MIN_TERM_ROWS)
  {
    drawTooSmall(ts);
    return;
  }

  auto infoOpt = audio.getCurrentTrackInfo();
  if (!infoOpt)
    return;

  const auto& info    = *infoOpt;
  const auto  backend = audio.getBackendInfo();

  const size_t size    = audio.getPlaylistSize();
  const size_t current = audio.getCurrentIndex();

  const auto curMeta = audio.getMetadataAt(current);
  if (!curMeta)
    return;

  const auto pi = utils::index::prevWrap(current, size);
  const auto ni = utils::index::nextWrap(current, size);

  // Only store titles for prev/next
  Title prevTitle = "<none>";
  Title nextTitle = "<none>";

  if (pi)
  {
    if (const auto pm = audio.getMetadataAt(*pi))
      prevTitle = pm->title;
  }

  if (ni)
  {
    if (const auto nm = audio.getMetadataAt(*ni))
      nextTitle = nm->title;
  }

  if (!curMeta)
    return;

  constexpr int BAR_W = 50;
  int filled          = info.lengthSec > 0.0 ? int((info.positionSec / info.lengthSec) * BAR_W) : 0;

  auto cfg = m_cfg.get();

  const auto& kb     = cfg->kb;
  const auto& colors = cfg->colors;

  utils::string::SmallString seekKeys;
  seekKeys += kb.seekBack->c_str();
  seekKeys += "/";
  seekKeys += kb.seekFwd->c_str();

  utils::string::SmallString volKeys;
  volKeys += kb.volUp->c_str();
  volKeys += "/";
  volKeys += kb.volDown->c_str();

  // ------------------------------------------------------------
  // UI print helpers
  // ------------------------------------------------------------

  auto l_Title = [&](std::string_view s) -> void
  { std::cout << colors::Ansi::Bold << colors.accent << s << colors::Ansi::Reset << "\n"; };

  auto l_Section = [&](std::string_view s) -> void
  { std::cout << colors::Ansi::Bold << colors.accent << " " << s << colors::Ansi::Reset << "\n"; };

  auto l_Sep = [&]() -> void
  {
    std::cout << colors::Ansi::Dim << colors.fg
              << "──────────────────────────────────────────────────────────" << colors::Ansi::Reset
              << "\n\n";
  };

  auto l_KvValue = [&](std::string_view k) -> void
  { std::cout << colors::Ansi::Bold << colors.fg << k << colors::Ansi::Reset; };

  auto l_KeyTag = [&](std::string_view keyName) -> void
  { std::cout << colors.accent << "[" << keyName << "]" << colors::Ansi::Reset; };

  auto l_Control3 = [&](std::string_view k1, std::string_view l1, std::string_view k2,
                        std::string_view l2, std::string_view k3, std::string_view l3) -> void
  {
    std::cout << "   ";
    l_KeyTag(k1);
    std::cout << " " << l1 << "   ";
    l_KeyTag(k2);
    std::cout << " " << l2 << "   ";
    l_KeyTag(k3);
    std::cout << " " << l3 << "\n";
  };

  std::cout << colors::Ansi::Clear;

  // base theme
  std::cout << colors.bg << colors.fg;

  l_Title(APP_TITLE);
  l_Sep();

  l_Section("Playlist");
  std::cout << "   Prev : " << prevTitle << "\n";
  std::cout << " " << colors.accent << "▶" << colors::Ansi::Reset << " Now  : ";
  l_KvValue(curMeta->title);
  std::cout << colors::Ansi::Reset << " — " << curMeta->artist << " (" << current + 1 << "/" << size
            << ")\n";

  std::cout << "   Next : " << nextTitle << "\n\n";

  l_Section("Track");
  std::cout << "   Album   : " << curMeta->album << "\n";
  std::cout << "   Artist  : " << curMeta->artist << "\n";
  std::cout << "   Year    : " << curMeta->year << "\n";
  std::cout << "   Track   : " << curMeta->track << " / " << curMeta->trackTotal << "\n";
  std::cout << "   Genre   : " << curMeta->genre << "\n";
  std::cout << "   Bitrate : " << curMeta->bitrate << " kbps\n";
  std::cout << "   File    : " << curMeta->filePath.c_str() << "\n\n";

  l_Section("Playback");
  std::cout << "   State   : " << (info.playing ? "Playing" : "Paused") << "\n";
  std::cout << "   Time    : " << utils::fmtTime(info.positionSec) << " / "
            << utils::fmtTime(info.lengthSec) << "\n";
  std::cout << "   Volume  : " << std::setw(3) << int(audio.getVolume() * 100.0f) << "%\n\n";

  std::cout << "   ";
  for (int i = 0; i < BAR_W; ++i)
  {
    if (i < filled)
      std::cout << colors.success << UI_BAR_FILL;
    else
      std::cout << colors.fg << UI_BAR_EMPTY;
  }
  std::cout << colors::Ansi::Reset << "\n\n";

  l_Section("Backend");
  std::cout << "   Device   : " << backend.dev.name.c_str() << "\n";
  std::cout << "   Codec    : " << backend.codecLongName.c_str() << " - ("
            << backend.codecName.c_str() << ")\n";
  std::cout << "   Format   : " << backend.pcmFormatName.c_str() << "\n";
  std::cout << "   Rate     : " << backend.sampleRate << " Hz\n";
  std::cout << "   Channels : " << backend.channels << "\n";
  std::cout << "   Buffer   : " << backend.bufferSize << " frames (" << std::fixed
            << std::setprecision(1) << backend.latencyMs << " ms)\n";
  std::cout << "   XRuns    : " << backend.xruns << "\n\n";

  l_Section("Controls");
  l_Control3(kb.playPause->c_str(), "play/pause", kb.restartTrack->c_str(), "restart",
             kb.randomTrack->c_str(), "random");
  l_Control3(kb.nextTrack->c_str(), "next", kb.prevTrack->c_str(), "prev", seekKeys.c_str(),
             "seek");
  l_Control3(volKeys.c_str(), "vol", kb.searchTitle->c_str(), "search title", kb.quit->c_str(),
             "quit");

  if (m_mode == UiMode::SearchTitle || m_mode == UiMode::SearchArtist)
  {
    drawBottomPrompt(ts, colors, "Search: ", m_searchBuf.c_str());
    return;
  }

  std::cout << colors::Ansi::Reset;
  std::cout.flush();
}

auto Interface::handleSearchTitleMode(audio::Service& audio, char c) -> bool
{
  return handleSearchCommon(
    c,
    [&]() -> bool
    {
      auto song = query::songmap::read::findSongObjByTitleFuzzy(*m_songMapTS, m_searchBuf);

      if (!song)
      {
        m_mode = UiMode::Normal;
        return true;
      }

      audio.clearPlaylist();
      auto h = audio.registerTrack(*song);
      audio.addToPlaylist(h);

      audio.nextTrack();
      m_mprisService->updateMetadata();
      m_mprisService->notify();

      query::songmap::read::forEachSongInAlbum(
        *m_songMapTS, audio.getCurrentMetadata()->artist, audio.getCurrentMetadata()->album,
        [&](const Disc&, const Track&, const ino_t, const Song& song) -> void
        {
          if (song.metadata.track <= audio.getCurrentMetadata()->track)
            return;

          auto h = audio.registerTrack(song);
          audio.addToPlaylist(h);
        });

      m_mode = UiMode::Normal;
      return true;
    });
}

auto Interface::handleSearchArtistMode(audio::Service& audio, char c) -> bool
{
  return handleSearchCommon(
    c,
    [&]() -> bool
    {
      if (m_searchBuf.empty())
      {
        m_mode = UiMode::Normal;
        return true;
      }

      constexpr size_t MAX_DIST = 3;
      const Artist     bestArtist =
        query::songmap::read::findArtistFuzzy(*m_songMapTS, Artist{m_searchBuf}, MAX_DIST);

      if (bestArtist.empty())
      {
        m_mode = UiMode::Normal;
        return true;
      }

      audio.clearPlaylist();

      bool queuedAny = false;

      query::songmap::read::forEachSong(
        *m_songMapTS,
        [&](const Artist& artist, const Album&, Disc, Track, ino_t, const Song& song) -> void
        {
          if (artist != bestArtist)
            return;

          auto h = audio.registerTrack(song);
          audio.addToPlaylist(h);
          queuedAny = true;
        });

      if (!queuedAny)
      {
        m_mode = UiMode::Normal;
        return true;
      }

      audio.restart();
      m_mprisService->updateMetadata();
      m_mprisService->notify();

      m_mode = UiMode::Normal;
      return true;
    });
}

auto Interface::handleKey(audio::Service& audio, char c) -> bool
{

  if (m_mode == UiMode::SearchTitle)
    return handleSearchTitleMode(audio, c);

  if (m_mode == UiMode::SearchArtist)
    return handleSearchArtistMode(audio, c);

  bool trackChanged = false;

  auto        cfg = m_cfg.get();
  const auto& kb  = cfg->kb;

  if (c == kb.playPause)
  {
    audio.isPlaying() ? audio.pauseCurrent() : audio.playCurrent();
  }
  else if (c == kb.nextTrack)
  {
    audio.nextTrack();
    trackChanged = true;
  }
  else if (c == kb.prevTrack)
  {
    audio.previousTrack();
    trackChanged = true;
  }
  else if (c == kb.randomTrack)
  {
    audio.randomTrack();
    trackChanged = true;
  }
  else if (c == kb.restartTrack)
  {
    audio.restartCurrent();
  }
  else if (c == kb.seekFwd)
  {
    m_pendingSeek += cfg->misc.seekDuration;
    return true;
  }
  else if (c == kb.seekBack)
  {
    m_pendingSeek -= cfg->misc.seekDuration;
    return true;
  }
  else if (c == kb.searchTitle)
  {
    m_mode = UiMode::SearchTitle;
    m_searchBuf.clear();
    return true;
  }
  else if (c == kb.searchArtist)
  {
    m_mode = UiMode::SearchArtist;
    m_searchBuf.clear();
    return true;
  }
  else if (c == kb.volUp)
  {
    audio.setVolume(std::min(1.5f, audio.getVolume() + 0.05f));
    return true;
  }
  else if (c == kb.volDown)
  {
    audio.setVolume(std::max(0.0f, audio.getVolume() - 0.05f));
    return true;
  }
  else if (c == kb.quit)
  {
    return false;
  }

  if (trackChanged)
    m_mprisService->updateMetadata();

  m_mprisService->notify();
  return true;
}

} // namespace frontend::cmdline
