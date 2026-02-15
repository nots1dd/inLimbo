#include "frontend/ftxui/state/album_art/Impl.hpp"
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace frontend::tui::state::album_art
{

using namespace ftxui;

void AlbumArtState::load(const std::string& path)
{
  if (loaded)
  {
    pixels.clear();
    loaded = false;
  }

  int desired_channels = 3;

  unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, desired_channels);

  if (!data)
  {
    loaded = false;
    return;
  }

  if (w <= 0 || h <= 0)
  {
    stbi_image_free(data);
    loaded = false;
    return;
  }

  size_t data_size = w * h * desired_channels;
  pixels.resize(data_size);
  std::memcpy(pixels.data(), data, data_size);

  stbi_image_free(data);
  loaded = true;
}

auto AlbumArtState::getAverageColor(int start_x, int start_y, int width, int height) -> Color
{
  if (!loaded || width <= 0 || height <= 0)
    return Color::Black;

  long r_sum = 0;
  long g_sum = 0;
  long b_sum = 0;
  int  count = 0;

  int end_x = std::min(start_x + width, w);
  int end_y = std::min(start_y + height, h);

  start_x = std::max(0, start_x);
  start_y = std::max(0, start_y);

  for (int y = start_y; y < end_y; ++y)
  {
    for (int x = start_x; x < end_x; ++x)
    {
      int idx = (y * w + x) * 3;

      r_sum += pixels[idx + 0];
      g_sum += pixels[idx + 1];
      b_sum += pixels[idx + 2];

      count++;
    }
  }

  if (count == 0)
    return Color::Black;

  return Color::RGB(static_cast<uint8_t>(r_sum / count), static_cast<uint8_t>(g_sum / count),
                    static_cast<uint8_t>(b_sum / count));
}

auto AlbumArtState::render() -> ftxui::Element
{
  auto term = Terminal::Size();

  const int constraint_w = term.dimx / 2;
  const int constraint_h = term.dimy - 4;

  if (!loaded)
  {
    return text("No Cover") | borderRounded | size(WIDTH, EQUAL, constraint_w) |
           size(HEIGHT, EQUAL, constraint_h) | center;
  }

  int virtual_max_h = constraint_h * 2;

  float scale_w = (float)constraint_w / w;
  float scale_h = (float)virtual_max_h / h;
  float scale   = std::min(scale_w, scale_h);

  int render_w = static_cast<int>(w * scale);
  int render_h = static_cast<int>(h * scale);

  if (render_h % 2 != 0)
    render_h--;

  if (render_w <= 0 || render_h <= 0)
    return text("Image too small");

  Elements rows;

  float src_block_w = (float)w / render_w;
  float src_block_h = (float)h / render_h;

  for (int y = 0; y < render_h; y += 2)
  {
    Elements row_cells;

    int horizontal_padding = (constraint_w - render_w) / 2;

    if (horizontal_padding > 0)
      row_cells.push_back(text(std::string(horizontal_padding, ' ')));

    for (int x = 0; x < render_w; ++x)
    {
      int src_x     = static_cast<int>(x * src_block_w);
      int src_y_top = static_cast<int>(y * src_block_h);
      int src_y_bot = static_cast<int>((y + 1) * src_block_h);

      int w = std::max(1, static_cast<int>(src_block_w));
      int h = std::max(1, static_cast<int>(src_block_h));

      Color top_color = getAverageColor(src_x, src_y_top, w, h);

      Color bot_color = getAverageColor(src_x, src_y_bot, w, h);

      row_cells.push_back(text("▀") | color(top_color) | bgcolor(bot_color));
    }

    rows.push_back(hbox(row_cells));
  }

  int used_rows = render_h / 2;
  int remaining = constraint_h - used_rows;

  for (int p = 0; p < remaining; ++p)
  {
    rows.push_back(text(std::string(constraint_w, ' ')));
  }

  return vbox(rows) | center;
}

} // namespace frontend::tui::state::album_art
