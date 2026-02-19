#include "frontend/raylib/ui/Fonts.hpp"
#include "frontend/raylib/Constants.hpp"

namespace frontend::raylib::ui
{

void Fonts::load()
{
  int  count = 0;
  int* cps   = LoadCodepoints(UI_GLYPHS, &count);

  regular = LoadFontEx("assets/fonts/SpaceMonoNerdFont-Regular.ttf", 32, cps, count);
  bold    = LoadFontEx("assets/fonts/SpaceMonoNerdFont-Bold.ttf", 34, cps, count);

  SetTextureFilter(regular.texture, TEXTURE_FILTER_TRILINEAR);
  SetTextureFilter(bold.texture, TEXTURE_FILTER_TRILINEAR);

  UnloadCodepoints(cps);
}

void Fonts::unload()
{
  UnloadFont(regular);
  UnloadFont(bold);
}

} // namespace frontend::raylib::ui
