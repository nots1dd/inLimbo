#include "frontend/cmd-line/Interface.hpp"

#include <cmath>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <unistd.h>

#include "mpris/Service.hpp"
#include "query/SongMap.hpp"
#include "utils/timer/Timer.hpp"

namespace frontend::cmdline
{

static constexpr int MIN_TERM_COLS = 80;
static constexpr int MIN_TERM_ROWS = 24;

#define UI_CLEAR     "\033[H\033[J"
#define UI_TITLE     "\033[1;36mInLimbo Player\033[0m"
#define UI_BAR_FILL  "█"
#define UI_BAR_EMPTY "░"

void Interface::run(audio::Service& audio)
{

  query::songmap::read::forEachSong(
    m_songMapTS,
    [&](const Artist&, const Album&, const Disc, const Track, const ino_t, const Song& song) -> void
    {
      auto h = audio.registerTrack(song);
      audio.addToPlaylist(h);
    });

  {
    std::lock_guard<std::mutex> lock(m_metaMutex);
    m_currentMeta = audio.getCurrentMetadata();
  }

  m_mprisService->updateMetadata();

  enableRawMode();
  m_isRunning.store(true);

  std::thread status([&]() -> void { statusLoop(audio); });
  std::thread input([&]() -> void { inputLoop(audio); });
  std::thread seek([&]() -> void { seekLoop(audio); });

  while (m_isRunning.load())
  {
    m_mprisService->poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
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
    draw(audio);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
  }
}

void Interface::drawTooSmall(const TermSize& ts)
{
  std::cout << UI_CLEAR << UI_TITLE << "\n\n";
  std::cout << " Terminal too small\n\n";
  std::cout << " Required : " << MIN_TERM_COLS << " cols × " << MIN_TERM_ROWS << " rows\n";
  std::cout << " Current  : " << ts.cols << " cols × " << ts.rows << " rows\n\n";
  std::cout << " Resize the terminal window.\n";
  std::cout.flush();
}

void Interface::draw(audio::Service& audio)
{
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

  auto curMeta  = audio.getMetadataAt(current);
  auto prevMeta = size ? audio.getMetadataAt((current + size - 1) % size) : std::nullopt;
  auto nextMeta = size ? audio.getMetadataAt((current + 1) % size) : std::nullopt;

  if (!curMeta)
    return;

  constexpr int BAR_W = 50;
  int filled          = info.lengthSec > 0.0 ? int((info.positionSec / info.lengthSec) * BAR_W) : 0;

  std::cout << UI_CLEAR << UI_TITLE << "\n";
  std::cout << "──────────────────────────────────────────────────────────\n\n";

  std::cout << " Playlist\n";
  std::cout << "   Prev : " << (prevMeta ? prevMeta->title : "<none>") << "\n";
  std::cout << " ▶ Now  : " << curMeta->title << " — " << curMeta->artist << " (" << current + 1
            << "/" << size << ")\n";
  std::cout << "   Next : " << (nextMeta ? nextMeta->title : "<none>") << "\n\n";

  std::cout << " Track\n";
  std::cout << "   Album   : " << curMeta->album << "\n";
  std::cout << "   Genre   : " << curMeta->genre << "\n";
  std::cout << "   Bitrate : " << curMeta->bitrate << " kbps\n";
  std::cout << "   File    : " << curMeta->filePath.c_str() << "\n\n";

  std::cout << " Playback\n";
  std::cout << "   State   : " << (info.playing ? "Playing" : "Paused") << "\n";
  std::cout << "   Time    : " << utils::fmtTime(info.positionSec) << " / "
            << utils::fmtTime(info.lengthSec) << "\n";
  std::cout << "   Volume  : " << std::setw(3) << int(audio.getVolume() * 100.0f) << "%\n\n";

  std::cout << "   ";
  for (int i = 0; i < BAR_W; ++i)
    std::cout << (i < filled ? UI_BAR_FILL : UI_BAR_EMPTY);
  std::cout << "\n\n";

  std::cout << " Backend\n";
  std::cout << "   Device   : " << backend.dev.description.c_str() << "\n";
  std::cout << "   Format   : " << backend.pcmFormatName << "\n";
  std::cout << "   Rate     : " << backend.sampleRate << " Hz\n";
  std::cout << "   Channels : " << backend.channels << "\n";
  std::cout << "   Buffer   : " << backend.bufferSize << " frames (" << std::fixed
            << std::setprecision(1) << backend.latencyMs << " ms)\n";
  std::cout << "   XRuns    : " << backend.xruns << "\n\n";

  std::cout << " Controls\n";
  std::cout << "   [p] play   [s] pause   [r] restart\n";
  std::cout << "   [n] next   [P] prev    [b/f] seek\n";
  std::cout << "   [+/-] vol  [q] quit\n";

  std::cout.flush();
}

auto Interface::handleKey(audio::Service& audio, char c) -> bool
{
  bool trackChanged = false;

  switch (c)
  {
    case 'p':
      audio.playCurrent();
      break;
    case 's':
      audio.pauseCurrent();
      break;
    case 'n':
      audio.nextTrack();
      trackChanged = true;
      break;
    case 'P':
      audio.previousTrack();
      trackChanged = true;
      break;
    case 'r':
      audio.restartCurrent();
      break;
    case 'b':
      m_pendingSeek -= 2.0;
      return true; // no notify yet
    case 'f':
      m_pendingSeek += 2.0;
      return true;
    case '=':
      audio.setVolume(std::min(1.5f, audio.getVolume() + 0.05f));
      return true;
    case '-':
      audio.setVolume(std::max(0.0f, audio.getVolume() - 0.05f));
      return true;
    case 'q':
      return false;
  }

  if (trackChanged)
    m_mprisService->updateMetadata();

  m_mprisService->notify();
  return true;
}

void Interface::showMetadata(const Metadata& m)
{
  std::cout << UI_CLEAR << UI_TITLE << "\n\n"
            << "Song    : " << m.title << " by " << m.artist << "\n"
            << "Album   : " << m.album << " (" << m.genre << ")\n"
            << "Bitrate : " << m.bitrate << " kbps\n"
            << "Path    : " << m.filePath.c_str() << "\n\n";
}

} // namespace frontend::cmdline
