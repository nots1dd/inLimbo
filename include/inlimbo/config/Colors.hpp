#pragma once

#include "Logger.hpp"
#include "config/Bind.hpp"
#include "config/Config.hpp"
#include "utils/Colors.hpp"
#include <functional>
#include <string>

namespace config::colors
{

using Member = utils::string::SmallString;

enum class Layer : ui8
{
  Foreground,
  Background
};

enum class Mode : ui8
{
  TrueColor24,
  Color256,
  Basic16
};

enum class Policy : ui8
{
  Adaptive, // don't hard force RGB; respect terminal theme
  Forced    // always emit absolute colors
};

// will check if apply is required or not,
// this is just a dummy bind so we are leaving
// it empty.
//
// in actuality, we are using ANSI scheme to bind
inline auto bind(std::string_view key, utils::colors::RGBA& out)
  -> config::colors::Binding<utils::colors::RGBA>
{
  return {.key = key, .target = &out, .apply = {}};
}

inline auto bindAnsi(std::string_view key, std::string& out, colors::Layer layer, colors::Mode mode)
  -> colors::Binding<std::string>
{
  return {.key    = key,
          .target = &out,
          .apply  = [=](const utils::colors::RGBA& c, std::string& dst) -> void
          {
            using A = utils::colors::Ansi;
            if (mode == Mode::TrueColor24)
              dst = (layer == Layer::Foreground) ? A::fgTrue(c) : A::bgTrue(c);
            else if (mode == Mode::Color256)
              dst = (layer == Layer::Foreground) ? A::fg256(toAnsi256(c)) : A::bg256(toAnsi256(c));
            else
              dst = A::Reset;
          }};
}

class ConfigLoader
{
public:
  explicit ConfigLoader(std::string_view frontend);

  template <typename... Bindings>
  auto load(Bindings&&... bindings) const -> void
  {
    if (!Config::isLoaded())
      throw std::runtime_error("Colors::ConfigLoader: toml not loaded");

    const auto& root = Config::table("colors");
    const auto* node = root.get(m_frontend);
    if (!node)
      return;

    const auto* tbl = node->as_table();
    if (!tbl)
      return;

    using Handler = std::function<void(const ::toml::node&)>;
    ankerl::unordered_dense::map<std::string_view, Handler> handlers;

    (registerBinding(handlers, std::forward<Bindings>(bindings)), ...);

    for (const auto& [k, v] : *tbl)
    {
      auto it = handlers.find(k.str());
      if (it != handlers.end())
        it->second(v);
      else
        LOG_WARN("Unknown color key: colors.{}.{}", m_frontend, k.str());
    }
  }

private:
  std::string m_frontend;

  template <typename T>
  static auto
  registerBinding(ankerl::unordered_dense::map<std::string_view,
                                               std::function<void(const ::toml::node&)>>& handlers,
                  const Binding<T>&                                                       b) -> void
  {
    handlers[b.key] = [ptr = b.target, apply = b.apply, key = b.key](const ::toml::node& n) -> auto
    {
      auto str = n.value<std::string>();
      if (!str)
      {
        LOG_WARN("Color '{}' must be string", key);
        return;
      }

      auto rgba = utils::colors::parseRGBA(*str);
      if (!rgba)
      {
        LOG_WARN("Invalid color '{}'", key);
        return;
      }

      if constexpr (std::is_same_v<T, utils::colors::RGBA>)
      {
        *ptr = *rgba;
      }
      else
      {
        // non-RGBA targets must provide apply()
        if (!apply)
        {
          LOG_ERROR("Color binding '{}' requires custom apply() for target type", key);
          return;
        }
        apply(*rgba, *ptr);
      }
    };
  }
};

} // namespace config::colors
