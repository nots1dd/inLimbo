#pragma once

#include "InLimbo-Types.hpp"
#include "audio/Service.hpp"
#include "frontend/Plugin.hpp"
#include "mpris/Service.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"

// Frontend Interface
//
// At a first glance, this does not look like a regular interface class
// as this is bound by a C ABI that ensures no bullshit code can be loaded
// or be executed just by overriding the interface classes methods.
//
// I also wanted frontend API versioning which seemed better like this.
//
// So all the core logic on inLimbo ends up here after
// main context initialization. (check include/inlimbo/Context.hpp)
//
// We require 3 main things from the main context:
//
// 1. Song Map (threads::Safe<>) -> Obviously for querying the library itself
// 2. Telemetry context          -> To update and showcase telemetry progress within frontend
// 3. MPRIS service              -> To poll, notify and update metadata sent to DBUS
//
// 1.1 Writing a frontend
//
// - Codebase Structure
//
// Currently inLimbo works as a monorepo for all frontends as well it is easier for me to
// traverse and make changes and as such, frontend code is expected to be integrated the 
// same way (but is not a compulsion)
//
// ideal tree:
// include/inlimbo/frontend/<your-frontend> -> define headers here:
//    - Interface.hpp
//    - Structs.hpp
// 
// src/frontend/<your-frontend>             -> define source files here:
//    - Interface.cc
//    - PluginShim.cc
//
// This may not be clear at first so I will do my best to explain in the following sections.
//
// 1.1.1 What do I need to do in Interface.hpp?
//
// Well, follow the base frontend interface class and its public methods.
//
// It is highly recommend to check out the `cmdline` and `ftxui` frontends,
// as they are mature, upto date and give a clear idea on how to write frontend
// code that inLimbo will accept.
//
// But in essence, you will require to do the following:
//
// -> Have a namespace `frontend::<your-frontend-name>` and ensure ALL your logic
// is contained within it.
//
// -> Declare `Interface` class with same constructor, public methods as the base class
// (Interface class is advised to be IMMUTABLE - non copyable & non movable)
//
// -> `run` is the main entry point of your frontend's execution
//
// -> `ready -> bool` is used by context to check if the frontend instance is ready.
//
// How does it know it is ready?
//
// Well within the ABI there is a `fe_create` function that just initializes the ctor.
// Essentially creates a new object of the class that is obviously regarded as a void ptr.
// If that works out with all symbols present and loaded via dlfcn, and the instance
// void ptr is regarded as not a nullptr, it is considered to be ready.
//
// Similarly, `fe_destroy` just deletes the object and memory is cleared, the main logic
// now goes back to the main context thread that launched the frontend. Its job now is to
// completely exit the app cleanly, save telemetry data and registry and handle any and
// all errors.
//
// Note that you DO NOT need to implement `ready` and `destroy`, you are required to only
// write the `run` function.
//
// -> private and protected methods are where your logic comes to play
//
// 1.1.2 PluginShim and ABI
//
// There is a decently strict ABI followed here that is implemented in C
// (check include/inlimbo/c/frontend_api.h)
//
// But it is abstracted down to just a PluginShim that you will need to call in a cpp file:
//
// Just call `INLIMBO_DEFINE_FRONTEND_PLUGIN_SHIM(FRONTEND_NS)` in a `PluginShim.cc`:
//
// ```cpp
// #include "frontend/PluginShim.hpp"
//
// INLIMBO_DEFINE_FRONTEND_PLUGIN_SHIM(frontend::cmdline)
// ```
//
// The above is an example of how the cmdline frontend implements the C ABI rules to create a
// successful plugin. It is easy and safe (imo) and the PluginError will throw any errors
// in your frontend neatly that is readable for easy debugging.
//
// The only disadvantage of this method is that over the C ABI, we have to pass all parameters
// of the constructor as raw pointers so C++ purists maybe a bit upset with this, but if handled well
// it is of trivial concern.
//
// 1.1.3 What is Structs.hpp?
//
// Although it is not necessary, for reloadable config within the frontend, it is
// advised to have `config::Watcher` class to efficiently poll for changes (via inotify)
// and update them within the frontend in real time.
//
// All implemented frontends (cmdline, ftxui, raylib) currently implement this, do look
// at the code for better understanding.
//
// Structs.hpp is the place to initialize your respective frontends configuration and
// fallbacks for the following:
//
// 1. Keybinds
// 2. Colors
// 3. Song map sorting
//
// I will be writing a more detailed doc on creating this (check include/inlimbo/frontend/cmdline/Structs.hpp).
//
// Well this is where your config handling will come into picture. 
//
// 1.2 Combining your code with CMake
//
// The number of frontends and their basic info is gathered at compile time via CMake. It is easy
// to integrate (check out templates/cmake/frontend/Frontend.cmake) for easy reference.
//
// The comments in that file neatly give the outline of what is expected to be changed by you in 
// order to make it work. Again if you face any problems, check out cmake/frontends/cmdline.cmake
// or ftxui.cmake files.
//
// After adding your cmake file, just include it in the main CMakeLists.txt:
//
// ```cmake
// include(${CMAKE_SOURCE_DIR}/cmake/frontends/ftxui.cmake)
// ```
//
// Note that this will dispatch ALL compile frontends to $HOME/.local/share/inLimbo/
// ($XDG_DATA_HOME env variable) and load it from there only!
//
// 2. Common Plugin Errors thrown
//
//  - Could not find symbol "X::X(Y, Z)..." -> Likely means that you did not compile a cpp file
//                                             that is part of your frontend source code.
//
//  - Plugin path not found -> Did not integrate with cmake pipeline properly.
//
//  - ABI mismatch -> Did not invoke plugin shim and likely tried to implement interface on your own?
//                                            
//  
// 3. End
// 
// The frontend interface logic is pretty stable as of writing this and I do not plan any big
// changes on this as it works pretty well and I do not have to touch a lot of the codebase now
// when making frontend changes.
//
// Despite that, if you have any issues or suggestions on making this model better do open an issue
// at https://github.com/nots1dd/inLimbo/issues, I appreciate it.
//

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
