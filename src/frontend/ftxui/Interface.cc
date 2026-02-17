#include "frontend/ftxui/Interface.hpp"
#include "query/SongMap.hpp"
#include "utils/timer/Timer.hpp"

using namespace ftxui;

namespace frontend::tui
{

constexpr int TITLE_WIDTH = 32;

static auto marquee(std::string_view text, int width, int tick) -> std::string
{
  if ((int)text.size() <= width)
    return std::string(text);

  const int span = (int)text.size() - width;
  const int t    = tick / 2; // scroll speed
  const int pos  = span == 0 ? 0 : t % (2 * span);

  const int offset = pos < span ? pos : (2 * span - pos);

  return std::string(text.substr(offset, width));
}

static auto vuMeter(float volume, int bars = 10) -> Element
{
  volume = std::clamp(volume, 0.0f, 1.5f);

  const int filled = static_cast<int>(volume / 1.5f * bars);

  Elements elems;
  for (int i = 0; i < bars; ++i)
  {
    Color c = i < filled ? (i < bars * 0.6    ? Color::Green
                            : i < bars * 0.85 ? Color::YellowLight
                                              : Color::RedLight)
                         : Color::GrayDark;

    elems.push_back(text("▮") | color(c));
  }

  return hbox(elems);
}

Interface::Interface(threads::SafeMap<SongMap>* songMap, telemetry::Context* telemetry,
                     mpris::Service* mpris)
    : m_songMapTS(songMap), m_telemetryCtx(telemetry), m_mpris(mpris),
      m_threadManager(m_mpris, m_songMapTS, m_telemetryCtx), m_libraryState(songMap),
      m_mainScreen(m_libraryState), m_nowPlayingScreen(m_nowState, m_albumArtState),
      m_queueScreen(m_queueState),
      m_eventHandler(active_screen, m_libraryState, m_nowState, m_queueState, m_threadManager,
                     m_songMapTS, m_mpris)
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
  m_queueScreen.attachAudioService(m_audio->get());

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
    case UIScreen::Queue:
      return vbox({renderTitleBar(), m_queueScreen.render() | flex,
                   renderStatusBar() | size(HEIGHT, EQUAL, 3)});

    default:
      return text("");
  }
}

auto Interface::renderTitleBar() -> Element
{
  std::string screen_name;
  Color       screen_color = Color::White;

  switch (active_screen)
  {
    case UIScreen::Main:
      screen_name  = "Library";
      screen_color = Color::GreenLight;
      break;

    case UIScreen::NowPlaying:
      screen_name  = "Now Playing";
      screen_color = Color::CyanLight;
      break;

    case UIScreen::Queue:
      screen_name  = "Queue";
      screen_color = Color::MagentaLight;
      break;

    default:
      break;
  }

  return hbox({
           text(" inLimbo ") | bold,
           text(" "),
           text(screen_name) | bold | color(screen_color),
           filler(),
         }) |
         bgcolor(Color::Blue) | color(Color::White);
}

auto Interface::renderStatusBar() -> Element
{
  switch (statusBarMode())
  {
    case StatusBarMode::Full:
      return renderStatusBarFull();

    case StatusBarMode::Reduced:
      return renderStatusBarReduced();

    case StatusBarMode::Compact:
      return renderStatusBarCompact();
  }

  return text("");
}

auto Interface::renderStatusBarFull() -> Element
{
  if (!m_audio)
    return text("");

  auto& audio = m_audio->get();

  auto meta      = audio.getCurrentMetadata();
  auto trackInfo = audio.getCurrentTrackInfo();
  auto backend   = audio.getBackendInfo();

  if (!meta || !trackInfo)
    return text("");

  float progress = 0.f;
  if (meta->duration > 0)
    progress = (float)trackInfo->positionSec / meta->duration;

  constexpr int bar_width = 34;
  int           filled    = static_cast<int>(progress * bar_width);

  static int tick = 0;
  tick++;

  int glow_pos = filled > 0 ? (tick / 2) % filled : 0;

  Elements bar;
  for (int i = 0; i < bar_width; ++i)
  {
    if (i < filled)
    {
      if (std::abs(i - glow_pos) <= 1)
        bar.push_back(text("▰") | color(Color::GreenLight));
      else
        bar.push_back(text("▰") | color(Color::Green));
    }
    else
    {
      bar.push_back(text("▱") | color(Color::GrayDark));
    }
  }

  auto progress_bar = hbox(bar);

  Element play_icon;
  if (audio.isPlaying())
  {
    // subtle pulse
    play_icon = text("▶") | color((tick % 20 < 10) ? Color::GreenLight : Color::Green) | bold;
  }
  else
  {
    play_icon = text("⏸") | color(Color::RedLight) | bold;
  }

  Element health = backend.common.xruns > 0 ? text("⚠") | color(Color::RedLight)
                                            : text("✓") | color(Color::GreenLight);

  return hbox({
           text(" "),
           play_icon,
           text("  "),
           text("♪") | color(Color::Cyan),
           text(" "),
           text(marquee(meta->title, TITLE_WIDTH, tick)) | bold,
           text("  ·  ") | dim,
           text(meta->artist) | color(Color::Cyan),
           text("  ·  ") | dim,
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
           health,
           text(" "),
           text(std::to_string(meta->bitrate) + " kbps") | dim | color(Color::Cyan),
           text("  "),
           separatorLight(),
           text("  "),
           text("Vol") | dim,
           text(" "),
           vuMeter(audio.getVolume(), 10),
           text(" "),
           text(std::to_string(int(audio.getVolume() * 100))) | dim,
           text("  "),
         }) |
         bgcolor(Color::RGB(18, 18, 18)) | borderRounded;
}

auto Interface::renderStatusBarReduced() -> Element
{
  if (!m_audio)
    return text("");

  auto& audio     = m_audio->get();
  auto  meta      = audio.getCurrentMetadata();
  auto  trackInfo = audio.getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("");

  static int tick = 0;
  tick++;

  return hbox({
           text(" "),
           audio.isPlaying() ? text("▶") | color(Color::GreenLight) | bold
                             : text("⏸") | color(Color::RedLight) | bold,
           text(" "),
           text(marquee(meta->title, 24, tick)) | bold,
           text(" · ") | dim,
           text(meta->artist) | color(Color::Cyan),
           text("  "),
           separatorLight(),
           text(" "),
           text(utils::timer::fmtTime(trackInfo->positionSec)) | color(Color::GreenLight),
           text("/") | dim,
           text(utils::timer::fmtTime(meta->duration)) | dim,
           filler(),
           vuMeter(audio.getVolume(), 7),
           text(" "),
         }) |
         bgcolor(Color::RGB(18, 18, 18)) | borderRounded;
}

auto Interface::renderStatusBarCompact() -> Element
{
  if (!m_audio)
    return text("");

  auto& audio     = m_audio->get();
  auto  meta      = audio.getCurrentMetadata();
  auto  trackInfo = audio.getCurrentTrackInfo();

  if (!meta || !trackInfo)
    return text("");

  static int tick = 0;
  tick++;

  return hbox({
           text(" "),
           audio.isPlaying() ? text("▶") | color(Color::GreenLight)
                             : text("⏸") | color(Color::RedLight),
           text(" "),
           text(marquee(meta->title, 18, tick)) | bold,
           filler(),
           text(utils::timer::fmtTime(trackInfo->positionSec)) | dim,
           text(" "),
         }) |
         bgcolor(Color::RGB(18, 18, 18)) | borderRounded;
}

} // namespace frontend::tui
