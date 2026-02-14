#pragma once

#include "InLimbo-Types.hpp"
#include "frontend/Plugin.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"

namespace audio
{
class Service;
}
namespace mpris
{
class Service;
}

namespace frontend
{

class Interface
{
public:
  Interface(Plugin& plugin, threads::SafeMap<SongMap>& map, telemetry::Context& telemetry,
            mpris::Service* mpris)
      : m_fePlugin(plugin)
  {
    m_instanceVPtr = m_fePlugin.create(&map, &telemetry, mpris);
    m_created      = (m_instanceVPtr != nullptr);
    if (!m_instanceVPtr)
      throw std::runtime_error("frontend::Interface: Failed to create frontend!");
  }

  [[nodiscard]] auto ready() const noexcept -> bool { return m_created; }

  // NOTE: This ASSUMES you have already checked for ready!
  void run(audio::Service& audio) { m_fePlugin.run(m_instanceVPtr, &audio); }

  void destroy()
  {
    if (m_created && m_instanceVPtr)
    {
      m_fePlugin.destroy(m_instanceVPtr);
      m_instanceVPtr = nullptr;
      m_created      = false;
    }
  }

private:
  Plugin& m_fePlugin;
  void*   m_instanceVPtr{nullptr};
  bool    m_created{false};
};

} // namespace frontend
