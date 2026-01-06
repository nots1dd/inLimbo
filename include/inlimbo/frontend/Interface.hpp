#pragma once

#include "InLimbo-Types.hpp"
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
  explicit Interface(threads::SafeMap<SongMap>& songMap, mpris::Service* mprisService = nullptr);

  void run(audio::Service& audio);

private:
  struct Impl; // opaque
  Impl* impl_; // type-erased, defined in .cc
};

} // namespace frontend

#define INLIMBO_DEFINE_FRONTEND_INTERFACE(frontend_ns)                                   \
  namespace frontend                                                                     \
  {                                                                                      \
  struct Interface::Impl                                                                 \
  {                                                                                      \
    frontend_ns::Interface impl;                                                         \
                                                                                         \
    Impl(threads::SafeMap<SongMap>& songMap, mpris::Service* mprisService)               \
        : impl(songMap, mprisService)                                                    \
    {                                                                                    \
    }                                                                                    \
  };                                                                                     \
                                                                                         \
  Interface::Interface(threads::SafeMap<SongMap>& songMap, mpris::Service* mprisService) \
      : impl_(new Impl(songMap, mprisService))                                           \
  {                                                                                      \
  }                                                                                      \
                                                                                         \
  void Interface::run(audio::Service& audio) { impl_->impl.run(audio); }                 \
  }
