#include "config/colors/ConfigLoader.hpp"

#include "Logger.hpp"
#include "config/Colors.hpp"
#include "toml/Parser.hpp"

#include <stdexcept>

namespace config::colors
{

ConfigLoader::ConfigLoader(std::string frontend) : m_frontend(std::move(frontend)) {}

auto ConfigLoader::loadIntoRegistry(bool overwriteExisting) const -> void
{
  if (!tomlparser::Config::isLoaded())
    throw std::runtime_error("ConfigLoader: tomlparser::Config not loaded");

  Theme t(m_frontend);

  try
  {
    const auto& colorsTbl = tomlparser::Config::table("colors");

    // [colors.<frontend>] table
    const auto* frontendTblNode = colorsTbl.get(m_frontend);
    if (!frontendTblNode)
    {
      LOG_ERROR("Colors theme not found: [colors.{}]", m_frontend);
      return;
    }

    const auto* frontendTbl = frontendTblNode->as_table();
    if (!frontendTbl)
    {
      LOG_ERROR("Colors section is not a table: [colors.{}]", m_frontend);
      return;
    }

    for (const auto& [k, node] : *frontendTbl)
    {
      const auto key = std::string(k.str());

      if (auto v = node.value<std::string>())
      {
        const auto col = fromHex(*v, RGBA{});

        if (v->empty() || ((*v)[0] != '#' && v->size() != 6 && v->size() != 8))
        {
          LOG_ERROR("Invalid color format: [colors.{}].{} = '{}'", m_frontend, key, *v);
          continue;
        }

        t.set(key, col);
      }
      else
      {
        LOG_ERROR("Invalid color type (expected string): [colors.{}].{}", m_frontend, key);
      }
    }
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Failed to load colors theme [colors.{}]: {}", m_frontend, e.what());
    return;
  }

  if (!overwriteExisting)
  {
    if (Registry::theme(m_frontend).name() == m_frontend)
      return;
  }

  Registry::put(std::move(t));
  Registry::setActive(m_frontend);

  LOG_DEBUG("Loaded colors theme [colors.{}] into Registry", m_frontend);
}

} // namespace config::colors
