#pragma once

namespace frontend::raylib::util
{

inline auto smooth(float cur, float tgt, float spd) -> float { return cur + (tgt - cur) * spd; }

} // namespace frontend::raylib::util
