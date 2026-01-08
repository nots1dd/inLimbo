#pragma once

#include "InLimbo-Types.hpp"
#include "frontend/Plugin.hpp"
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
  Interface(Plugin& plugin, threads::SafeMap<SongMap>& map, mpris::Service* mpris)
      : m_fePlugin(plugin)
  {
    m_instanceVPtr = m_fePlugin.create(&map, mpris);
    if (!m_instanceVPtr)
      throw std::runtime_error("Failed to create frontend");
  }

  ~Interface() { m_fePlugin.destroy(m_instanceVPtr); }

  void run(audio::Service& audio) { m_fePlugin.run(m_instanceVPtr, &audio); }

private:
  Plugin& m_fePlugin;
  void*   m_instanceVPtr;
};

} // namespace frontend
