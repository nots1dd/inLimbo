#include "frontend/ftxui/ui/screens/Help.hpp"
#include <ftxui/component/component.hpp>

using namespace ftxui;

namespace frontend::tui::ui::screens
{

HelpScreen::HelpScreen(state::help::HelpState& helpState)
{
  content = Renderer(
    [&]() -> Element
    {
      Elements    out;
      const auto& elems = helpState.elements();

      for (int i = 0; i < (int)elems.size(); ++i)
      {
        Element row = elems[i];

        if (i == helpState.selected())
          row = row | bgcolor(Color::RGB(40, 60, 90)) | color(Color::White) | bold;
        else
          row = row | dim;

        out.push_back(row);
      }

      return vbox(out) | vscroll_indicator;
    });

  view = Renderer(content,
                  [&]() -> Element
                  {
                    const int count = (int)helpState.elements().size();
                    if (count > 1)
                      scroll = float(helpState.selected()) / float(count - 1);
                    else
                      scroll = 0.0f;

                    return content->Render() | focusPositionRelative(0.0f, scroll) | frame | flex;
                  });
}

auto HelpScreen::component() -> Component { return view; }

auto HelpScreen::render() -> Element
{
  return window(text(" Help ") | bold, view->Render() | frame | flex) |
         borderStyled(BorderStyle::HEAVY, Color::Aquamarine1);
}

} // namespace frontend::tui::ui::screens
