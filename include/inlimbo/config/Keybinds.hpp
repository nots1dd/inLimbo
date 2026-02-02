#pragma once

#include "Logger.hpp"
#include "config/Bind.hpp"
#include "config/KBUtils.hpp"
#include "toml/Parser.hpp"
#include <functional>
#include <string>
#include <string_view>

namespace config::keybinds
{

class ConfigLoader
{
public:
  explicit ConfigLoader(std::string_view frontend);

  template <typename... Bindings> auto load(Bindings&&... bindings) const -> void
  {
    if (!tomlparser::Config::isLoaded())
      throw std::runtime_error("Keybinds::ConfigLoader: toml not loaded");

    const auto& rootTbl = tomlparser::Config::table("keybinds");

    const auto* frontendNode = rootTbl.get(m_frontend);
    if (!frontendNode)
      return;

    const auto* tbl = frontendNode->as_table();
    if (!tbl)
      return;

    using Node    = toml::node;
    using Handler = std::function<void(const Node&)>;

    ankerl::unordered_dense::map<std::string_view, Handler> handlers;
    (registerBinding(handlers, std::forward<Bindings>(bindings)), ...);

    for (const auto& [k, node] : *tbl)
    {
      const std::string_view key = k.str();

      if (auto it = handlers.find(key); it != handlers.end())
        it->second(node);
      else
        LOG_WARN("Unknown keybind: keybinds.{}.{}", m_frontend, key);
    }
  }

private:
  std::string m_frontend;

  template <typename T>
  static auto
  registerBinding(ankerl::unordered_dense::map<std::string_view,
                                               std::function<void(const toml::node&)>>& handlers,
                  const keybinds::Binding<T>&                                           b) -> void
  {
    handlers[b.key] = [ptr = b.target, nameKey = b.key,
                       apply = b.apply](const toml::node& n) -> auto
    {
      if (auto v = n.value<std::string>())
      {
        auto ch = parseSingleChar(*v);
        if (!ch)
        {
          LOG_WARN("Invalid keybind '{}'", nameKey);
          return;
        }

        // custom apply func for key and target
        if (apply)
          apply(*ch, *ptr);
        else
        {
          // default behavior
          ptr->key  = *ch;
          ptr->name = parseKeyName(*ch);
        }
      }
      else
      {
        LOG_WARN("Keybind '{}' must be string", nameKey);
      }
    };
  }
};

} // namespace config::keybinds
