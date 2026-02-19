#pragma once

namespace frontend::raylib::state
{

struct UI
{
  enum class Screen
  {
    Library,
    NowPlaying
  };

  Screen screen       = Screen::Library;
  bool   showMetaInfo = false;
  float  screenBlend  = 0.0f;
  bool   running      = true;
};

} // namespace frontend::raylib::state
