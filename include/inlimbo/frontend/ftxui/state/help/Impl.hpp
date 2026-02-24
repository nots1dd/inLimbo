#pragma once

#include <ftxui/dom/elements.hpp>
#include <vector>

#include "frontend/ftxui/Structs.hpp"
#include "utils/ClassRulesMacros.hpp"

namespace frontend::tui::state::help
{

class HelpState
{
public:
  DEFAULT_CTOR(HelpState);

  void rebuild(const TuiConfig& cfg);
  void moveSelection(int delta);

  [[nodiscard]] auto elements() const -> const std::vector<ftxui::Element>& { return m_elements; }

  [[nodiscard]] auto selected() const -> int { return m_selected; }

private:
  std::vector<ftxui::Element> m_elements;
  std::vector<int>            m_selectable;
  int                         m_selected = 0;
};

} // namespace frontend::tui::state::help
