#include "scroller.hpp"

/**
 * @file scroller.hpp
 * @brief Implementation of a custom Scroller component for FTXUI.
 */

namespace ftxui
{
/**
 * @class ScrollerBase
 * @brief A custom scroller component that synchronizes with an external index and applies a custom
 * cursor background color.
 *
 * The `ScrollerBase` component disables internal navigation and allows the scroll state
 * to be controlled externally using an external index (`external_selected_`).
 * It also supports custom styling for the focused element using a background color (`cursor_bg_`).
 */
class ScrollerBase : public ComponentBase
{
public:
  /**
   * @brief Constructor for the ScrollerBase class.
   *
   * @param child The child component to be rendered inside the scroller.
   * @param external_selected A pointer to an external integer that controls the selected index.
   * @param cursor_bg The background color to apply to the focused element.
   * @param inactive_menu_cursor_bg The background color to apply to the inactive menu elements.
   * @param current_playing_col The background color to apply to the currently playing element.
   * @param current_playing_index The index of the currently playing element.
   */
  ScrollerBase(Component child, int* external_selected, Color cursor_bg,
               Color inactive_menu_cursor_bg)
      : external_selected_(external_selected), cursor_bg_(cursor_bg),
        inactive_menu_cursor_bg_(inactive_menu_cursor_bg)
  {
    Add(child);
  }

private:
  /**
   * @brief Renders the scroller component.
   *
   * This function synchronizes the internal selected index with the external index
   * and applies the custom cursor background color to the focused element.
   *
   * @return The rendered FTXUI Element.
   */
  auto Render() -> Element final
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

  /**
   * @brief Handles events for the scroller component.
   *
   * This implementation ignores all keyboard and mouse events, disabling
   * internal navigation and relying entirely on external control.
   *
   * @param event The event to handle.
   * @return Always returns `false` since no events are handled.
   */
  auto OnEvent(Event event) -> bool final
  {
    // Ignore all keyboard/mouse events to disable internal navigation
    return false;
  }

  /**
   * @brief Determines if the scroller is focusable.
   *
   * @return Always returns `true` since the scroller can be focused.
   */
  [[nodiscard]] auto Focusable() const -> bool final { return true; }

  int   selected_ = 0;            ///< The currently selected index.
  int   size_     = 0;            ///< The size of the child component.
  int*  external_selected_;       ///< Pointer to the external selected index.
  Color cursor_bg_;               ///< Background color for the focused element.
  Color inactive_menu_cursor_bg_; ///< Background color for the current element which is unfocused.
  Box   box_;                     ///< The bounding box of the scroller.
};

/**
 * @brief Factory function to create a Scroller component.
 *
 * This function creates an instance of `ScrollerBase` with external index control and
 * a custom cursor background color.
 *
 * @param child The child component to be rendered inside the scroller.
 * @param external_selected A pointer to an external integer that controls the selected index.
 * @param cursor_bg The background color to apply to the focused element.
 * @param inactive_menu_cursor_bg The background color to apply to the inactive menu elements.
 * @param current_playing_col The background color to apply to the currently playing element.
 * @param current_playing_index The index of the currently playing element.
 * @return A `Component` instance representing the scroller.
 */
auto Scroller(Component child, int* external_selected, Color cursor_bg,
              Color inactive_menu_cursor_bg) -> Component
{
  return Make<ScrollerBase>(std::move(child), external_selected, cursor_bg,
                            inactive_menu_cursor_bg);
}

} // namespace ftxui
