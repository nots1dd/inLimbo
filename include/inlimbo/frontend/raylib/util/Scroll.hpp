#pragma once

namespace frontend::raylib::util
{

void ensureVisible(int index, float itemHeight, float viewHeight, float contentHeight,
                   float& scrollY);

} // namespace frontend::raylib::util
