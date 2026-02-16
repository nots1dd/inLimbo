#pragma once

#include "audio/Service.hpp"
#include "frontend/ftxui/state/queue/Impl.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::tui::ui::screens
{

class QueueScreen
{
public:
  explicit QueueScreen(state::queue::QueueState& state);

  auto component() -> ftxui::Component;
  auto render() -> ftxui::Element;

  void attachAudioService(audio::Service& audio) { m_audioPtr = &audio; }

private:
  audio::Service*           m_audioPtr;
  state::queue::QueueState& m_state;

  ftxui::Component queue_content;
  ftxui::Component meta_content;
  ftxui::Component container;
};

} // namespace frontend::tui::ui::screens
