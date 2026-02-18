#include "frontend/ftxui/ui/screens/Queue.hpp"
#include "utils/fs/FileUri.hpp"
#include "utils/timer/Timer.hpp"
#include <ftxui/component/event.hpp>

using namespace ftxui;

namespace frontend::tui::ui::screens
{

static auto yesno(bool v) -> std::string { return v ? "yes" : "no"; }

QueueScreen::QueueScreen(state::queue::QueueState& state) : m_state(state)
{
  queue_content = Renderer(
    [&]() -> Element
    {
      Elements rows;

      const size_t size    = m_audioPtr->getPlaylistSize();
      const size_t current = m_audioPtr->getCurrentIndex();

      for (size_t i = 0; i < size; ++i)
      {
        auto        metaOpt = m_audioPtr->getMetadataAt(i);
        std::string title   = metaOpt ? metaOpt->title : "<unknown>";

        Element row = hbox({
          text(i == current ? "▶ " : "  "),
          text(title),
        });

        if ((int)i == m_state.selected())
          row = row | bgcolor(Color::RGB(40, 60, 90)) | color(Color::White) | bold;

        rows.push_back(row);
      }

      if (rows.empty())
        rows.push_back(text("Queue is empty") | dim | center);

      return vbox(rows);
    });

  queue_view = Renderer(
    queue_content,
    [&]() -> Element
    {
      const size_t size = m_audioPtr->getPlaylistSize();

      if (size > 1)
      {
        queue_scroll_target = float(m_state.selected()) / float(std::max<size_t>(1, size - 1));
      }

      // ---- smoothing ----
      constexpr float smoothing = 0.15f;
      queue_scroll += (queue_scroll_target - queue_scroll) * smoothing;

      if (std::abs(queue_scroll_target - queue_scroll) < 0.001f)
        queue_scroll = queue_scroll_target;

      queue_scroll = std::clamp(queue_scroll, 0.0f, 1.0f);

      return queue_content->Render() | focusPositionRelative(0.0f, queue_scroll) | frame | flex;
    });

  meta_content = Renderer(
    [&]() -> Element
    {
      auto metaOpt = m_audioPtr->getMetadataAt(m_state.selected());
      auto infoOpt = m_audioPtr->getCurrentTrackInfo();
      auto backend = m_audioPtr->getBackendInfo();

      if (!metaOpt)
      {
        return vbox({
                 text("No track selected") | dim | center,
               }) |
               flex;
      }

      const auto& m = *metaOpt;

      Elements rows;

      rows.push_back(text("Track") | bold | color(Color::Cyan));
      rows.push_back(text("Title     : " + m.title));
      rows.push_back(text("Artist    : " + m.artist));
      rows.push_back(text("Album     : " + m.album));
      rows.push_back(text("Genre     : " + m.genre));
      rows.push_back(text("Disc      : " + std::to_string(m.discNumber)));
      rows.push_back(text("Track #   : " + std::to_string(m.track)));
      rows.push_back(text("Year      : " + std::to_string(m.year)));
      rows.push_back(text("Duration  : " + utils::timer::fmtTime(m.duration)));
      rows.push_back(text("Bitrate   : " + std::to_string(m.bitrate) + " kbps"));
      rows.push_back(text("HasLyrics : " + yesno(!m.lyrics.empty())));

      rows.push_back(separator());

      rows.push_back(text("File") | bold | color(Color::Cyan));
      rows.push_back(text("Path:") | bold);
      rows.push_back(text(m.filePath) | dim);
      rows.push_back(text("Cache Art File Path:") | bold);
      if (!m.artUrl.empty())
        rows.push_back(text(utils::fs::fromAbsFilePathUri(m.artUrl).c_str()) | dim);
      else
        rows.push_back(text("<none>") | dim);

      rows.push_back(separator());

      rows.push_back(text("Backend (Common)") | bold | color(Color::Cyan));
      rows.push_back(text(std::string("Device   : ") + backend.common.dev.name.c_str()));
      rows.push_back(text("Latency  : " + std::to_string((int)backend.common.latencyMs) + " ms"));
      rows.push_back(text("XRuns    : " + std::to_string(backend.common.xruns)));
      rows.push_back(text("Writes   : " + std::to_string(backend.common.writes)));
      rows.push_back(text("Active   : " + yesno(backend.common.isActive)));

      rows.push_back(separator());

      rows.push_back(text("Backend Details") | bold | color(Color::Cyan));

      std::visit(
        [&](const auto& info)
        {
          using T = std::decay_t<decltype(info)>;

          if constexpr (std::is_same_v<T, audio::backend::AlsaBackendInfo>)
          {
            rows.push_back(text("Type     : ALSA"));
            rows.push_back(text(std::string("Format   : ") + info.pcmFormatName.c_str()));
            rows.push_back(text("Period   : " + std::to_string(info.periodSize) + " frames"));
            rows.push_back(text("Buffer   : " + std::to_string(info.bufferSize) + " frames"));
            rows.push_back(text(std::string("Draining : ") + yesno(info.isDraining)));
          }
          else
          {
            rows.push_back(text("Type     : <unknown>") | dim);
          }
        },
        backend.specific);

      rows.push_back(separator());
      rows.push_back(text("Decoder") | bold | color(Color::Cyan));

      if (!backend.common.codecLongName.empty())
        rows.push_back(text(std::string("Name     : ") + backend.common.codecLongName.c_str()));
      else if (!backend.common.codecName.empty())
        rows.push_back(text(std::string("Name     : ") + backend.common.codecName.c_str()));
      else
        rows.push_back(text("Name     : <unknown>") | dim);

      return vbox(rows) | flex;
    });
}

auto QueueScreen::component() -> Component { return container; }

auto QueueScreen::render() -> Element
{
  m_state.clamp(m_audioPtr->getPlaylistSize());

  auto term       = Terminal::Size();
  int  half_width = term.dimx / 2;

  auto queue_inner = window(text(" Queue ") | bold, queue_view->Render() | frame | flex) |
                     size(WIDTH, EQUAL, half_width);

  auto meta_inner = window(text(" Track Info ") | bold, meta_content->Render() | frame | flex) |
                    size(WIDTH, EQUAL, term.dimx - half_width);

  auto queue_pane = queue_inner | borderStyled(BorderStyle::HEAVY, Color::Green);

  auto meta_pane = meta_inner | borderStyled(BorderStyle::HEAVY, Color::GrayDark);

  return vbox({hbox({
                 queue_pane,
                 meta_pane,
               }) |
               flex});
}

} // namespace frontend::tui::ui::screens
