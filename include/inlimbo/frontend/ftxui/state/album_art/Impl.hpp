#pragma once

#include "utils/Colors.hpp"
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
  auto getAverageColor(int start_x, int start_y, int width, int height) -> utils::colors::RGBA;
  auto getDominantColor(int x, int y, int bw, int bh) -> utils::colors::RGBA;

  std::vector<unsigned char> pixels;
  int                        w{0};
  int                        h{0};
  int                        channels{0};
  bool                       loaded{false};
};

} // namespace frontend::tui::state::album_art
