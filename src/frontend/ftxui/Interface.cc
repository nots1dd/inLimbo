#include "frontend/ftxui/Interface.hpp"
#include "query/SongMap.hpp"
#include "utils/timer/Timer.hpp"

using namespace ftxui;

namespace frontend::tui
{

Interface::Interface(threads::SafeMap<SongMap>* songMap, telemetry::Context* telemetry,
                     mpris::Service* mpris)
    : m_songMapTS(songMap), m_telemetryCtx(telemetry), m_mpris(mpris),
      m_threadManager(m_mpris, m_songMapTS, m_telemetryCtx), m_libraryState(songMap),
      m_mainScreen(m_libraryState), m_nowPlayingScreen(m_nowState, m_albumArtState),
      m_eventHandler(active_screen, m_libraryState, m_nowState, m_threadManager, m_songMapTS,
                     m_mpris)
{
  m_libraryState.rebuild();
  renderer = CatchEvent(Renderer([&]() mutable -> Element { return renderRoot(); }),
                        [&](Event e) -> bool { return m_eventHandler.handle(e); });
}

auto Interface::getRoot() -> ftxui::Component { return renderer; }

void Interface::run(audio::Service& audio)
{
  m_audio = audio;

  m_threadManager.attachAudioService(m_audio->get());
  m_eventHandler.attachAudioService(m_audio->get());
  m_nowPlayingScreen.attachAudioService(m_audio->get());
  m_mainScreen.attachAudioService(m_audio->get());

  query::songmap::read::forEachSongInAlbum(
    *m_songMapTS, audio.getCurrentMetadata()->artist, audio.getCurrentMetadata()->album,
    [&](const Disc&, const Track&, const ino_t, const std::shared_ptr<Song>& song) -> void
    {
      if (song->metadata.track <= audio.getCurrentMetadata()->track)
        return;

      auto h = audio.registerTrack(song);
      audio.addToPlaylist(h);
    });

  m_mpris->updateMetadata();

  ScreenInteractive screen = ScreenInteractive::Fullscreen();

  m_screen = &screen;

  m_threadManager.setScreen(m_screen);

  m_threadManager.setOnConfigReload([this]() -> void
                                    { m_needsRebuild.store(true, std::memory_order_release); });

  m_threadManager.start();

  screen.Loop(getRoot());

  m_threadManager.stop();
}

auto Interface::renderRoot() -> Element
{
  if (m_needsRebuild.exchange(false))
  {
    m_libraryState.rebuild();
  }

  switch (active_screen)
  {
    case UIScreen::Main:
      return vbox({renderTitleBar(), m_mainScreen.render() | flex,
                   renderStatusBar() | size(HEIGHT, EQUAL, 3)});

    case UIScreen::NowPlaying:
      return vbox({renderTitleBar(), m_nowPlayingScreen.render() | flex,
                   renderStatusBar() | size(HEIGHT, EQUAL, 3)});

    default:
      return text("");
  }
}

auto Interface::renderTitleBar() -> Element
{
  return hbox({text(" inLimbo ") | bold, filler(), text("Modern TUI Player") | dim}) |
         bgcolor(Color::Blue) | color(Color::White);
}

auto Interface::renderStatusBar() -> Element
{
  if (!m_audio)
    return text("");

  auto meta      = m_audio->get().getCurrentMetadata();
  auto trackInfo = m_audio->get().getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("");

  float progress = 0.f;
  if (meta->duration > 0)
    progress = (float)trackInfo->positionSec / meta->duration;

  int bar_width = 36;
  int filled    = static_cast<int>(progress * bar_width);

  static int tick = 0;
  tick++;

  int glow_pos = 0;
  if (filled > 0)
    glow_pos = (tick / 2) % filled;

  Elements bar;

  for (int i = 0; i < bar_width; ++i)
  {
    if (i < filled)
    {
      if (std::abs(i - glow_pos) <= 1)
        bar.push_back(text("█") | color(Color::GreenLight) | bold);
      else
        bar.push_back(text("█") | color(Color::Green));
    }
    else
    {
      bar.push_back(text("░") | color(Color::GrayDark));
    }
  }

  auto progress_bar = hbox(bar);

  return hbox({text(" "),
               text(m_audio->get().isPlaying() ? " PLAY " : " PAUSE ") |
                 (m_audio->get().isPlaying() ? color(Color::GreenLight) : color(Color::RedLight)) |
                 bold,
               text("  "),
               separatorLight(),
               text("  "),
               text(meta->title) | bold,
               text("  •  ") | dim,
               text(meta->artist) | color(Color::Cyan),
               text("  •  ") | dim,
               text(meta->genre.empty() ? "Unknown" : meta->genre) | color(Color::MagentaLight),
               text("  "),
               separatorLight(),
               text("  "),
               progress_bar,
               text("  "),
               text(utils::timer::fmtTime(trackInfo->positionSec)) | color(Color::GreenLight),
               text(" / ") | dim,
               text(utils::timer::fmtTime(meta->duration)) | dim,
               filler(),
               text(std::to_string(meta->bitrate) + " kbps") | dim | color(Color::Cyan),
               text("  "),
               separatorLight(),
               text("  "),
               text("Vol ") | dim,
               text(std::to_string(static_cast<int>(m_audio->get().getVolume() * 100))) | dim,
               text(" "),
               gauge(m_audio->get().getVolume()) | size(WIDTH, EQUAL, 10) | color(Color::BlueLight),
               text("   ")}) |
         bgcolor(Color::RGB(18, 18, 18)) | borderRounded;
}

} // namespace frontend::tui
