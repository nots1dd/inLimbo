#pragma once

#include "Interface.hpp"
#include "c/mpris.h"

namespace mpris
{

class Service
{
public:
  Service(IMprisBackend& backend, const std::string& name);
  ~Service();

  void poll();
  void notify();

  void updateMetadata();

private:
  mpris_service* m_C_svc;
  IMprisBackend& m_IBackend;
};

} // namespace mpris
