#include "config/keybinds/ConfigLoader.hpp"

#include "Logger.hpp"
#include "config/Keybinds.hpp"
#include "toml/Parser.hpp"

#include <stdexcept>

namespace config::keybinds
{

ConfigLoader::ConfigLoader(std::string frontend) : m_frontend(std::move(frontend)) {}

auto ConfigLoader::loadIntoRegistry(bool overwriteExisting) const -> void
{
  if (!tomlparser::Config::isLoaded())
    throw std::runtime_error("Keybinds::ConfigLoader: tomlparser::Config not loaded");

  Theme t(m_frontend);

  try
  {
    const auto& rootTbl = tomlparser::Config::table("keybinds");

    // [keybinds.<frontend>] table
    const auto* frontendNode = rootTbl.get(m_frontend);
    if (!frontendNode)
    {
      LOG_ERROR("Keybinds theme not found: [keybinds.{}]", m_frontend);
      return;
    }

    const auto* frontendTbl = frontendNode->as_table();
    if (!frontendTbl)
    {
      LOG_ERROR("Keybinds section is not a table: [keybinds.{}]", m_frontend);
      return;
    }

    for (const auto& [k, node] : *frontendTbl)
    {
      const auto action = std::string(k.str());

      if (auto v = node.value<std::string>())
      {
        const auto ch = parseSingleChar(*v);
        if (!ch)
        {
          LOG_ERROR("Invalid keybind value: [keybinds.{}].{} = '{}'", m_frontend, action, *v);
          continue;
        }

        t.set(action, Keybind{*ch});
      }
      else
      {
        LOG_ERROR("Invalid keybind type (expected string): [keybinds.{}].{}", m_frontend, action);
      }
    }
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Failed to load keybinds theme [keybinds.{}]: {}", m_frontend, e.what());
    return;
  }

  if (!overwriteExisting)
  {
    if (Registry::theme(m_frontend).name() == m_frontend)
      return;
  }

  Registry::put(std::move(t));
  Registry::setActive(m_frontend);

  LOG_DEBUG("Loaded keybinds theme [keybinds.{}] into Registry", m_frontend);
}

} // namespace config::keybinds
