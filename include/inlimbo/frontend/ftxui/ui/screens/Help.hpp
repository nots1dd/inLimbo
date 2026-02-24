#pragma once

#include "frontend/ftxui/state/help/Impl.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace frontend::tui::ui::screens
{

class HelpScreen
{
public:
  HelpScreen(state::help::HelpState& helpState);

  auto component() -> ftxui::Component;
  auto render() -> ftxui::Element;

private:
  ftxui::Component content;
  ftxui::Component view;
  float            scroll = 0.0f;
};

} // namespace frontend::tui::ui::screens
