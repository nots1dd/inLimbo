#pragma once

#include "c/frontend_api.h"

// just needs the frontend namespace it is residing in
//
// ex: INLIMBO_DEFINE_FRONTEND_PLUGIN_SHIM(frontend::cmdline)
#define INLIMBO_DEFINE_FRONTEND_PLUGIN_SHIM(FRONTEND_NS)                                         \
  extern "C"                                                                                     \
  {                                                                                              \
                                                                                                 \
    static void* fe_create(void* songMap, void* telemetry, void* mpris)                          \
    {                                                                                            \
      if (!songMap)                                                                              \
        return nullptr;                                                                          \
                                                                                                 \
      return new FRONTEND_NS::Interface(static_cast<threads::SafeMap<SongMap>*>(songMap),        \
                                        static_cast<telemetry::Context*>(telemetry),             \
                                        static_cast<mpris::Service*>(mpris));                    \
    }                                                                                            \
                                                                                                 \
    static void fe_run(void* instance, void* audio)                                              \
    {                                                                                            \
      if (!instance || !audio)                                                                   \
        return;                                                                                  \
                                                                                                 \
      static_cast<FRONTEND_NS::Interface*>(instance)->run(*static_cast<audio::Service*>(audio)); \
    }                                                                                            \
                                                                                                 \
    static void fe_destroy(void* instance)                                                       \
    {                                                                                            \
      delete static_cast<FRONTEND_NS::Interface*>(instance);                                     \
    }                                                                                            \
                                                                                                 \
    static const InLimboFrontendAPI g_inlimbo_frontend_api = {                                   \
      .abi_version = INLIMBO_FRONTEND_ABI_VERSION,                                               \
      .struct_size = sizeof(InLimboFrontendAPI),                                                 \
      .create      = fe_create,                                                                  \
      .run         = fe_run,                                                                     \
      .destroy     = fe_destroy,                                                                 \
    };                                                                                           \
                                                                                                 \
    const InLimboFrontendAPI* inlimbo_frontend_get_api() { return &g_inlimbo_frontend_api; }     \
                                                                                                 \
  } /* extern "C" */
