#include "frontend/raylib/ui/TextUtils.hpp"

namespace frontend::raylib::ui::text
{

auto truncate(Font font, const char* text, float maxWidth, float fontSize, float spacing)
  -> std::string
{
  Vector2 size = MeasureTextEx(font, text, fontSize, spacing);
  if (size.x <= maxWidth)
    return text;

  std::string s(text);
  while (!s.empty())
  {
    s.pop_back();
    std::string e = s + "...";
    if (MeasureTextEx(font, e.c_str(), fontSize, spacing).x <= maxWidth)
      return e;
  }
  return "...";
}

void drawCentered(Font font, const char* text, Vector2 c, float fs, Color col)
{
  Vector2 sz = MeasureTextEx(font, text, fs, 1);
  DrawTextEx(font, text, {c.x - sz.x / 2, c.y - sz.y / 2}, fs, 1, col);
}

void drawTruncated(Font font, const char* text, Vector2 pos, float fs, float spacing, Color col,
                   float maxW)
{
  auto s = truncate(font, text, maxW, fs, spacing);
  DrawTextEx(font, s.c_str(), pos, fs, spacing, col);
}

void drawWrappedText(Font font, const std::string& text, Rectangle bounds, float fontSize,
                     float spacing, Color color)
{
  std::string line;
  float       y = bounds.y;

  size_t start = 0;
  while (start < text.size())
  {
    size_t end   = start;
    float  width = 0.0f;

    while (end < text.size())
    {
      std::string test = text.substr(start, end - start + 1);
      width            = MeasureTextEx(font, test.c_str(), fontSize, spacing).x;

      if (width > bounds.width)
        break;

      ++end;
    }

    if (end == start) // single character overflow safety
      ++end;

    line = text.substr(start, end - start);
    DrawTextEx(font, line.c_str(), {bounds.x, y}, fontSize, spacing, color);

    y += fontSize + 4;
    start = end;

    if (y > bounds.y + bounds.height)
      break;
  }
}

} // namespace frontend::raylib::ui::text
