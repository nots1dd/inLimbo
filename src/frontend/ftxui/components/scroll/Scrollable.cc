#include "frontend/ftxui/components/scroll/Scrollable.hpp"

namespace ftxui
{
class ScrollerBase : public ComponentBase
{
public:
  ScrollerBase(Component child, int* external_selected, Color cursor_bg,
               Color inactive_menu_cursor_bg)
      : external_selected_(external_selected), cursor_bg_(cursor_bg),
        inactive_menu_cursor_bg_(inactive_menu_cursor_bg)
  {
    Add(child);
  }

private:
  auto Render() -> Element
  {
    auto focused = Focused() ? focus : ftxui::select;

    // Use the cursor background color when focused
    auto style = Focused() ? color(cursor_bg_) : color(inactive_menu_cursor_bg_);

    auto inverted_style = Focused() ? inverted : inverted;

    // Sync selected_ with external_selected_
    selected_ = *external_selected_;

    Element background = ComponentBase::Render();
    background->ComputeRequirement();
    size_ = background->requirement().min_y;

    return dbox({
             std::move(background),
             vbox({
               text(L"") | size(HEIGHT, EQUAL, selected_),
               text(L"") | style | focused | inverted_style,
             }),
           }) |
           vscroll_indicator | yframe | yflex | reflect(box_);
  }

  auto OnEvent([[maybe_unused]] Event event) -> bool final
  {
    // Ignore all keyboard/mouse events to disable internal navigation
    return false;
  }

  [[nodiscard]] auto Focusable() const -> bool final { return true; }

  int   selected_ = 0;            ///< The currently selected index.
  int   size_     = 0;            ///< The size of the child component.
  int*  external_selected_;       ///< Pointer to the external selected index.
  Color cursor_bg_;               ///< Background color for the focused element.
  Color inactive_menu_cursor_bg_; ///< Background color for the current element which is unfocused.
  Box   box_;                     ///< The bounding box of the scroller.
};

auto Scroller(Component child, int* external_selected, Color cursor_bg,
              Color inactive_menu_cursor_bg) -> Component
{
  return Make<ScrollerBase>(std::move(child), external_selected, cursor_bg,
                            inactive_menu_cursor_bg);
}

} // namespace ftxui
