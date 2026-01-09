#include "frontend/raylib/util/Scroll.hpp"
#include <algorithm>

namespace frontend::raylib::util
{

void ensureVisible(int index, float ih, float vh, float ch, float& scroll)
{
  float top    = index * ih;
  float bottom = top + ih;

  float viewT = -scroll;
  float viewB = viewT + vh;

  if (top < viewT)
    scroll = -top;
  else if (bottom > viewB)
    scroll = -(bottom - vh);

  scroll = std::clamp(scroll, std::min(0.f, vh - ch), 0.f);
}

} // namespace frontend::raylib::util
