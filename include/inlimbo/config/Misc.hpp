#pragma once

#include "Logger.hpp"
#include "config/Bind.hpp"
#include "toml/Parser.hpp"
#include <functional>
#include <string>

namespace config::misc
{

class ConfigLoader
{
public:
  explicit ConfigLoader(std::string frontend) : m_frontend(std::move(frontend)) {}

  template <typename... Bindings> auto load(Bindings&&... bindings) const -> void
  {
    if (!tomlparser::Config::isLoaded())
      throw std::runtime_error("Misc::ConfigLoader: toml not loaded");

    const auto& rootTbl = tomlparser::Config::table("misc");

    const auto* frontendNode = rootTbl.get(m_frontend);
    if (!frontendNode)
      return;

    const auto* tbl = frontendNode->as_table();
    if (!tbl)
      return;

    using Node    = toml::node;
    using Handler = std::function<void(const Node&)>;

    std::unordered_map<std::string_view, Handler> handlers;

    (registerBinding(handlers, std::forward<Bindings>(bindings)), ...);

    // Dispatch
    for (const auto& [k, node] : *tbl)
    {
      const std::string_view key = k.str();

      if (auto it = handlers.find(key); it != handlers.end())
        it->second(node);
      else
        LOG_WARN("Unknown misc config key: misc.{}.{}", m_frontend, key);
    }
  }

private:
  std::string m_frontend;

  template <typename T>
  static auto registerBinding(
    std::unordered_map<std::string_view, std::function<void(const toml::node&)>>& handlers,
    const keybinds::Binding<T>&                                                   b) -> void
  {
    handlers[b.key] = [ptr = b.target, key = b.key](const toml::node& n) -> auto
    {
      if (auto v = n.value<T>())
        *ptr = *v;
      else
        LOG_WARN("misc key '{}' has wrong type", key);
    };
  }
};

} // namespace config::misc
