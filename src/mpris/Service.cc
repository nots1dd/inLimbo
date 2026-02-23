#include "mpris/Service.hpp"
#include "c/mpris.h"

namespace mpris
{

Service::Service(IMprisBackend& b, const std::string& appName) : m_IBackend(b)
{
  mpris_backend be{};
  be.userdata = &m_IBackend;

  /* Playback */
  be.play     = [](void* u) -> void { static_cast<IMprisBackend*>(u)->play(); };
  be.pause    = [](void* u) -> void { static_cast<IMprisBackend*>(u)->pause(); };
  be.stop     = [](void* u) -> void { static_cast<IMprisBackend*>(u)->stop(); };
  be.next     = [](void* u) -> void { static_cast<IMprisBackend*>(u)->next(); };
  be.previous = [](void* u) -> void { static_cast<IMprisBackend*>(u)->previous(); };

  /* Seeking */
  be.seek = [](void* u, double s) -> void { static_cast<IMprisBackend*>(u)->seekSeconds(s); };

  be.set_position = [](void* u, double s) -> void
  { static_cast<IMprisBackend*>(u)->setPositionSeconds(s); };

  /* State */
  be.is_playing = [](void* u) -> bool { return static_cast<IMprisBackend*>(u)->isPlaying(); };

  be.position = [](void* u) -> double { return static_cast<IMprisBackend*>(u)->positionSeconds(); };

  be.duration = [](void* u) -> double { return static_cast<IMprisBackend*>(u)->durationSeconds(); };

  be.refresh_metadata = [](void* u, mpris_service* svc) -> void
  {
    auto* b = static_cast<IMprisBackend*>(u);

    mpris_update_metadata(svc, b->title().c_str(), b->artist().c_str(), b->album().c_str(),
                          b->artUrl().c_str());
  };

  m_C_svc = mpris_create(appName.c_str(), be);

  /* Push initial metadata once */
  updateMetadata();
  notify();
}

Service::~Service() { mpris_destroy(m_C_svc); }

void Service::poll() { mpris_poll(m_C_svc); }

void Service::notify() { mpris_emit(m_C_svc); }

void Service::updateMetadata()
{
  mpris_update_metadata(m_C_svc, m_IBackend.title().c_str(), m_IBackend.artist().c_str(),
                        m_IBackend.album().c_str(), m_IBackend.artUrl().c_str());
}

} // namespace mpris
