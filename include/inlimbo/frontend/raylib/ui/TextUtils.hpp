#pragma once

#include <raylib.h>
#include <string>

namespace frontend::raylib::ui::text
{

auto truncate(Font font, const char* text, float maxWidth, float fontSize, float spacing = 1.0f)
  -> std::string;

void drawCentered(Font font, const char* text, Vector2 center, float fontSize, Color color);

void drawTruncated(Font font, const char* text, Vector2 pos, float fontSize, float spacing,
                   Color color, float maxWidth);

void drawWrappedText(Font font,
                            const std::string& text,
                            Rectangle bounds,
                            float fontSize,
                            float spacing,
                            Color color);

} // namespace frontend::raylib::ui::text
