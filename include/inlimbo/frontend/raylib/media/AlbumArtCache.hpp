#pragma once

#include "InLimbo-Types.hpp"
#include <raylib.h>
#include <string>

namespace frontend::raylib::media
{

class AlbumArtCache
{
public:
  void clear();
  auto get(const Metadata& meta) -> Texture2D*;

private:
  std::string m_url;
  Texture2D   m_tex{};
  bool        m_loaded = false;
};

} // namespace frontend::raylib::media
