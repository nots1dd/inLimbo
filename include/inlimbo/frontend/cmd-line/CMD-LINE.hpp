#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

#include "audio/Playback.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "thread/Map.hpp"

namespace frontend::cmdline
{

#define UI_CLEAR     "\033[H\033[J"
#define UI_TITLE     "\033[1;36mInLimbo Player\033[0m"
#define UI_PLAY      "▶"
#define UI_PAUSE     "⏸"
#define UI_BAR_FILL  "█"
#define UI_BAR_EMPTY "░"

class Interface
{
public:
  explicit Interface(threads::SafeMap<core::SongMap>& songMap) : m_songMapTS(std::move(songMap)) {}

  void run(audio::AudioEngine& eng, const Metadata& meta)
  {
    enableRawMode();
    m_isRunning.store(true);

    std::thread status([&]() -> void { statusLoop(eng, meta); });
    std::thread input([&]() -> void { inputLoop(eng); });
    std::thread seek([&]() -> void { seekLoop(eng); });

    while (m_isRunning.load())
      std::this_thread::sleep_for(std::chrono::milliseconds(40));

    m_isRunning.store(false);
    disableRawMode();

    if (input.joinable())
      input.join();
    if (status.joinable())
      status.join();
    if (seek.joinable())
      seek.join();
  }

  auto selectAudioDevice(const audio::Devices& devices) -> size_t
  {
    std::cout << "\nPlayback Devices:\n";
    for (size_t i = 0; i < devices.size(); ++i)
      std::cout << " [" << i << "] " << devices[i].description << "\n";

    size_t idx = 0;
    for (;;)
    {
      std::cout << "\nSelect device: ";
      if (std::cin >> idx && idx < devices.size())
        break;
      std::cin.clear();
      std::cin.ignore(1024, '\n');
    }
    std::cin.ignore(1024, '\n');
    return idx;
  }

private:
  threads::SafeMap<core::SongMap> m_songMapTS;
  std::atomic<bool>               m_isRunning{false};
  std::atomic<double>             m_pendingSeek{0.0};
  termios                         m_termOrig{};

  void enableRawMode()
  {
    tcgetattr(STDIN_FILENO, &m_termOrig);
    termios raw = m_termOrig;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  }

  void disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_termOrig); }

  void statusLoop(audio::AudioEngine& eng, const Metadata& met)
  {
    while (m_isRunning.load())
    {
      draw(eng, met);
      std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
  }

  void draw(const audio::AudioEngine& eng, const Metadata& met)
  {
    auto [pos, len]             = *eng.getPlaybackTime();
    float               vol     = eng.getVolume() * 100.0f;
    bool                play    = eng.isPlaying();
    const audio::Sound& songObj = eng.getSound(0);

    constexpr int W      = 50;
    int           filled = len > 0.0 ? int((pos / len) * W) : 0;

    showMetadataAndBackendInfo(met, songObj);

    std::cout << (play ? UI_PLAY : UI_PAUSE) << "  " << std::fixed << std::setprecision(1) << pos
              << " / " << len << " s   Vol " << std::setw(3) << int(vol) << "%\n\n";

    for (int i = 0; i < W; ++i)
      std::cout << (i < filled ? UI_BAR_FILL : UI_BAR_EMPTY);

    std::cout << "\n\n[p] play  [s] stop  [r] restart  [b/f] seek  [+/-] vol  [q] quit\n";
    std::cout.flush();
  }

  void inputLoop(audio::AudioEngine& eng)
  {
    pollfd pfd{.fd = STDIN_FILENO, .events = POLLIN, .revents = 0};
    while (m_isRunning.load())
    {
      if (poll(&pfd, 1, 25) > 0)
      {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0)
        {
          if (!handleKey(eng, c))
            break;
        }
      }
    }
    m_isRunning.store(false);
  }

  void seekLoop(audio::AudioEngine& eng)
  {
    while (m_isRunning.load())
    {
      double d = m_pendingSeek.exchange(0.0);
      if (std::abs(d) > 0.01)
      {
        if (d > 0.0)
          eng.seekForward(d);
        else
          eng.seekBackward(-d);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(70));
    }
  }

  auto handleKey(audio::AudioEngine& eng, char c) -> bool
  {
    switch (c)
    {
      case 'p':
        eng.play();
        break;
      case 's':
        eng.pause();
        break;
      case 'r':
        eng.restart();
        break;
      case 'b':
        m_pendingSeek -= 2.0;
        break;
      case 'f':
        m_pendingSeek += 2.0;
        break;
      case '=':
        eng.setVolume(std::min(1.5f, eng.getVolume() + 0.05f));
        break;
      case '-':
        eng.setVolume(std::max(0.0f, eng.getVolume() - 0.05f));
        break;
      case 'q':
        return false;
      default:
        break;
    }
    return true;
  }

  static void showMetadataAndBackendInfo(const Metadata& m, const audio::Sound& sound)
  {
    std::cout << UI_CLEAR << UI_TITLE << "\n\n"
              << "Song    : " << m.title << " by " << m.artist << "\n"
              << "Album   : " << m.album << " (" << m.genre << ")\n"
              << "Bitrate : " << m.bitrate << " kbps\n"
              << "Path    : " << m.filePath << "\n\n"
              << "Sample Rate : " << sound.source.sampleRate << " Hz\n"
              << "Channels    : " << sound.source.channels << "\n"
              << "Format      : " << sound.source.sampleFmtName << "\n\n"
              << "Press any key...\n";
    std::cout.flush();
  }
};

} // namespace frontend::cmdline
