#include "frontend/raylib/media/AlbumArtCache.hpp"

namespace frontend::raylib::media
{

void AlbumArtCache::clear()
{
  if (m_loaded)
  {
    UnloadTexture(m_tex);
    m_loaded = false;
    m_url.clear();
  }
}

auto AlbumArtCache::get(const Metadata& meta) -> Texture2D*
{
  if (meta.artUrl == m_url)
    return m_loaded ? &m_tex : nullptr;

  if (m_loaded)
  {
    UnloadTexture(m_tex);
    m_loaded = false;
  }

  m_url.clear();
  if (meta.artUrl.empty())
    return nullptr;

  const char* path =
    meta.artUrl.starts_with("file://") ? meta.artUrl.c_str() + 7 : meta.artUrl.c_str();

  Image img = LoadImage(path);
  if (!img.data)
    return nullptr;

  m_tex    = LoadTextureFromImage(img);
  m_loaded = true;
  m_url    = meta.artUrl;

  UnloadImage(img);
  return &m_tex;
}

} // namespace frontend::raylib::media
