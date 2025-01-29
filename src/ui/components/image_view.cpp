#include <memory> // for make_shared
#include <sstream>
#include <string>  // for string, wstring
#include <ftxui/dom/deprecated.hpp>  // for text, vtext
#include <ftxui/dom/elements.hpp>    // for Element, text, vtext
#include <ftxui/dom/node.hpp>        // for Node
#include <ftxui/dom/requirement.hpp> // for Requirement
#include <ftxui/screen/box.hpp>      // for Box
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp> // for Pixel, Screen
#include <ftxui/screen/string.hpp> // for string_width, Utf8ToGlyphs, to_string

#include "image_view.hpp"
#include "libs/tiv_lib.h"

namespace ftxui
{

namespace
{

using ftxui::Screen;

class ImageView : public Node
{
public:
  explicit ImageView(std::string_view url) : url_(url) {}

  void ComputeRequirement() override
  {
    img_ = tiv::load_rgb_CImg(url_.c_str());

    requirement_.min_x = img_.width() / 4;
    requirement_.min_y = img_.height() / 8;
  }

  void Render(Screen& screen) override
  {
    auto get_pixel = [this](int row, int col) -> unsigned long
    {
      return (((unsigned long)img_(row, col, 0, 0)) << 16) |
             (((unsigned long)img_(row, col, 0, 1)) << 8) | (((unsigned long)img_(row, col, 0, 2)));
    };
    auto origin_image_width  = (box_.x_max - box_.x_min + 1) * 4;
    auto origin_image_height = (box_.y_max - box_.y_min + 1) * 8;
    auto new_size =
      tiv::size(img_).fitted_within(tiv::size(origin_image_width, origin_image_height));
    img_.resize(new_size.width, new_size.height, -100, -100, 5);

    auto          flags = tiv::FLAG_24BIT;
    tiv::CharData lastCharData;
    auto          screen_y = box_.y_min;
    for (int y = 0; y <= img_.height() - 8; y += 8)
    {
      auto screen_x = box_.x_min;
      for (int x = 0; x <= img_.width() - 4; x += 4)
      {
        if (screen_x > box_.x_max)
          break;
        std::stringstream output;
        tiv::CharData     charData = tiv::findCharData(get_pixel, x, y, tiv::FLAG_24BIT);
        ftxui::Color      bgColor(charData.bgColor[0], charData.bgColor[1], charData.bgColor[2]);
        ftxui::Color      fgColor(charData.fgColor[0], charData.fgColor[1], charData.fgColor[2]);
        tiv::printCodepoint(output, charData.codePoint);
        auto pixel                           = ftxui::Pixel();
        pixel.background_color               = bgColor;
        pixel.foreground_color               = fgColor;
        pixel.character                      = output.str();
        screen.PixelAt(screen_x++, screen_y) = pixel;
      }
      ++screen_y;
    }
  }

private:
  std::string url_;

  cimg_library::CImg<unsigned char> img_;
  int                               width_;
  int                               height_;
};

} // namespace

Element image_view(std::string_view url) { return std::make_shared<ImageView>(url); }

} // namespace ftxui
