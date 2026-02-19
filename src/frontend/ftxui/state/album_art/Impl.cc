#include "frontend/ftxui/state/album_art/Impl.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/*
  TERMINAL ALBUM ART RENDERING LOGIC

  This renderer draws an image inside a terminal using text characters.
  A terminal cell cannot draw real pixels, so we fake pixels using color
  and special block characters.

  BASIC IDEA
  ----------
  - Each terminal cell represents TWO vertical image pixels.
  - The character '▀' draws the top half using the foreground color
    and the bottom half using the background color.
  - This gives double vertical resolution compared to normal text.

  IMAGE SCALING
  -------------
  - The terminal width and height are read at runtime.
  - The image is scaled so it fits inside the available space.
  - Height is doubled internally because one terminal row represents two image rows.
  - The scaled image size is clamped so it never overflows the terminal.

  PIXEL MAPPING
  -------------
  - For every output cell:
    - A small rectangular block of source pixels is selected.
    - Two vertical samples are taken:
      - one for the top half
      - one for the bottom half

  COLOR SAMPLING
  --------------
  Two different color sampling methods are used:

  1) Average color
     - Computes the mean RGB value of all pixels in the block.
     - Produces smooth gradients.
     - Reduces noise and harsh transitions.

  2) Dominant color
     - Samples a single representative pixel near the center.
     - Preserves sharp edges and fine details.
     - Can look pixelated if overused.

  ADAPTIVE BLENDING
  -----------------
  - The code compares the dominant top and bottom colors.
  - If the color difference is large:
      - The block is likely an edge or detail.
      - Dominant colors are used to keep it sharp.
  - If the difference is small:
      - The block is likely flat or smooth.
      - Averaged colors are used to avoid blockiness.

  CHARACTER CHOICE
  ----------------
  - '▀' is normally used to represent two vertical pixels.
  - '█' is used when the top and bottom colors are very similar,
    making the block appear solid and more uniform.

  FINAL OUTPUT
  ------------
  - Each row of cells is assembled into a horizontal box.
  - Rows are stacked vertically.
  - Extra blank rows are added to fill unused space.
  - The entire image is centered in the terminal.

  RESULT
  ------
  - Smooth areas remain smooth.
  - Edges stay sharp.
*/

// Why are we using unicodes '▀' (half block) and '█' (full block)
//
// Visually they fill up lesser space than any other character ive seen so we can
// place more of these characters to make it less pixelated and more detailed.
//
// ▀ lets one terminal cell represent two vertical pixels,
// and █ is used when those pixels are the same so the image looks cleaner
// instead of striped.
//

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

  int            desired_channels = 3;
  unsigned char* data             = stbi_load(path.c_str(), &w, &h, &channels, desired_channels);

  if (!data || w <= 0 || h <= 0)
  {
    if (data)
      stbi_image_free(data);
    loaded = false;
    return;
  }

  const size_t data_size = size_t(w) * size_t(h) * desired_channels;
  pixels.resize(data_size);
  std::memcpy(pixels.data(), data, data_size);

  stbi_image_free(data);
  loaded = true;
}

// takes in a color channel and tries to push it away from neutral (128)
//
// so dark colors -> darker, light colors -> lighter
static inline auto boostContrast(uint8_t c) -> uint8_t
{
  constexpr float factor = 1.15f;
  int             v      = int((c - 128) * factor + 128);
  return static_cast<uint8_t>(std::clamp(v, 0, 255));
}

auto AlbumArtState::getAverageColor(int start_x, int start_y, int width, int height)
  -> utils::colors::RGBA
{
  if (!loaded || width <= 0 || height <= 0)
    return {0, 0, 0, 255};

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
      ++count;
    }
  }

  if (count == 0)
    return {0, 0, 0, 255};

  return {static_cast<uint8_t>(r_sum / count), static_cast<uint8_t>(g_sum / count),
          static_cast<uint8_t>(b_sum / count), 255};
}

auto AlbumArtState::getDominantColor(int x, int y, int bw, int bh) -> utils::colors::RGBA
{
  if (!loaded || bw <= 0 || bh <= 0)
    return {0, 0, 0};

  const int cx = std::clamp(x + bw / 2, 0, w - 1);
  const int cy = std::clamp(y + bh / 2, 0, h - 1);

  const int idx = (cy * w + cx) * 3;

  return {boostContrast(pixels[idx + 0]), boostContrast(pixels[idx + 1]),
          boostContrast(pixels[idx + 2])};
}

auto AlbumArtState::render() -> Element
{
  auto term = Terminal::Size();

  const int constraint_w = term.dimx / 2;
  const int constraint_h = term.dimy;

  if (!loaded)
  {
    return text("No Cover") | borderRounded | size(WIDTH, EQUAL, constraint_w) |
           size(HEIGHT, EQUAL, constraint_h) | center;
  }

  const int virtual_max_h = constraint_h * 2;

  float scale_w = float(constraint_w) / float(w);
  float scale_h = float(virtual_max_h) / float(h);
  float scale   = std::min(scale_w, scale_h);

  int render_w = int(w * scale);
  int render_h = int(h * scale);

  if (render_h % 2 != 0)
    --render_h;

  if (render_w <= 0 || render_h <= 0)
    return text("Image too small");

  const float src_block_w = float(w) / float(render_w);
  const float src_block_h = float(h) / float(render_h);

  Elements rows;

  for (int y = 0; y < render_h; y += 2)
  {
    Elements row_cells;

    int pad = (constraint_w - render_w) / 2;
    if (pad > 0)
      row_cells.push_back(text(std::string(pad, ' ')));

    for (int x = 0; x < render_w; ++x)
    {
      int src_x     = int(x * src_block_w);
      int src_y_top = int(y * src_block_h);
      int src_y_bot = int((y + 1) * src_block_h);

      int bw = std::max(1, int(src_block_w));
      int bh = std::max(1, int(src_block_h));

      auto avg_top = getAverageColor(src_x, src_y_top, bw, bh);
      auto avg_bot = getAverageColor(src_x, src_y_bot, bw, bh);

      auto dom_top = getDominantColor(src_x, src_y_top, bw, bh);
      auto dom_bot = getDominantColor(src_x, src_y_bot, bw, bh);

      int diff =
        abs(dom_top.r - dom_bot.r) + abs(dom_top.g - dom_bot.g) + abs(dom_top.b - dom_bot.b);

      auto top = diff > 40 ? dom_top : avg_top;
      auto bot = diff > 40 ? dom_bot : avg_bot;

      row_cells.push_back(text(diff < 30 ? "█" : "▀") | color(Color::RGB(top.r, top.g, top.b)) |
                          bgcolor(Color::RGB(bot.r, bot.g, bot.b)));
    }

    rows.push_back(hbox(row_cells));
  }

  int used_rows = render_h / 2;
  for (int i = used_rows; i < constraint_h; ++i)
    rows.push_back(text(std::string(constraint_w, ' ')));

  return vbox(rows) | center;
}

} // namespace frontend::tui::state::album_art
