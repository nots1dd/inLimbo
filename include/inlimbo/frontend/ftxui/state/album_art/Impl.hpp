#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

namespace frontend::tui::state::album_art
{

class AlbumArtState
{
public:
  void load(const std::string& path);
  auto render() -> ftxui::Element;

private:
  auto getAverageColor(int x, int y, int w, int h) -> ftxui::Color;

  std::vector<unsigned char> pixels;
  int                        w{0};
  int                        h{0};
  int                        channels{0};
  bool                       loaded{false};
};

} // namespace frontend::tui::state::album_art
