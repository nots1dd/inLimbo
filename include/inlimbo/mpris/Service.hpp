#pragma once

#include "Interface.hpp"
#include "c/mpris.h"

// MPRIS Service
//
// The structure of how we get to this service is:
//
// ┌──────────────────────────────┐
// │          DBus / MPRIS        │
// │  (org.mpris.MediaPlayer2)    │
// └──────────────▲───────────────┘
//                │
//                │ calls / signals
//                │
// ┌──────────────┴───────────────┐
// │        mpris_service (C)     │  ← opaque handle
// │  - DBus glue                 │
// │  - State cache               │
// └──────────────▲───────────────┘
//                │
//                │ function pointers
//                │
// ┌──────────────┴───────────────┐
// │      mpris_backend (C)       │  ← C ABI boundary
// │  - userdata (IMprisBackend*) │
// │  - play(), pause(), title()  │
// └──────────────▲───────────────┘
//                │
//                │ dispatch
//                │
// ┌──────────────┴───────────────┐
// │      mpris::Service (C++)    │  ← CPP boundary
// │  - owns mpris_service*       │
// │  - owns backend reference    │
// └──────────────▲───────────────┘
//                │
//                │ virtual calls
//                │
// ┌──────────────┴───────────────┐
// │    IMprisBackend (C++)       │  ← Strategy interface
// │  - play(), seek(), metadata  │
// └──────────────▲───────────────┘
//                │
//                │ concrete impl
//                │
// ┌──────────────┴───────────────┐
// │     audio::Service calls     │
// └──────────────────────────────┘
//
// Why was the C ABI made?
//
// Initially, the choice was made due to a simple reason.
// I did not want to use gio or glib for this, it was too
// tacky and felt too much for a simple DBus connection
// and event poller/notifier.
//
// So i just wanted to write it in pure c using libdbus
// API directly instead and statically compile it as a
// separate library.
//
// An added benefit was that I can change the logic however
// I want when these events are handled in cpp boundary,
// without worrying about messing up the DBus core logic.
//
// But quite honestly, this isnt required still and I could
// have just written externed C code over to cpp itself,
// and had one core backend implementation (backend::Common)
//
// It currently is very stable and works pretty well, so
// I do not have any intention of changing this however.
// (unless some good points are made on why this is shitty)
//
// Also the frontend does not have to worry about all this,
// which is another added benefit of just abstracting it all
// behind an interface and language boundary.
//
// 1. How to create a MPRIS service?
//
// -> Create the backend implementation first
// (or just use mpris::backend::Common)
//
// Note that the backend must inherit `IMprisBackend`
// and ofc override all methods.
//
// -> Have an App name ready.
//
// Then simply invoke like so:
//
// ```cpp
// mpris::backend::Common mprisBackend(audioService);
// mpris::Service mprisService(mprisBackend, "test");
// ```
//
// FAQ:
//
// - Will I need to create new mpris service objs in
//   the frontend?
//
// NO. The inLimbo main context will handle it at compile
// time. Frontend will ONLY receive that mpris service
// as a pointer over the frontend C ABI.
//
// In short, you do not need to worry about creation
// or deletion of this object. The lifetime is strictly
// handled by the main context at all times.
//
// - Why would I ever need to create a custom MPRIS
//   backend?
//
// Maybe to add some logging, connect to lastfm API,
// idk. Possibilities with such a model is ofc quite
// extensive and I will leave it upto you to decide.

namespace mpris
{

class Service
{
public:
  Service(IMprisBackend& backend, const std::string& appName);
  ~Service();

  void poll();
  void notify();

  // this is for the inLimbo client to update metadata forcibly.
  //
  // DBus emit and interactions will automatically refresh as well.
  void updateMetadata();

private:
  mpris_service* m_C_svc;
  IMprisBackend& m_IBackend;
};

} // namespace mpris
