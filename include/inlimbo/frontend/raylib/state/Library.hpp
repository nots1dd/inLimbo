#pragma once

#include "InLimbo-Types.hpp"
#include <vector>

namespace frontend::raylib::state
{

struct Library
{
  std::vector<Artist> artists;
  std::vector<Album>  albums;

  int   selectedArtist = 0;
  float artistScrollY  = 0.0f;
  float albumScrollY   = 0.0f;
};

} // namespace frontend::raylib::state
