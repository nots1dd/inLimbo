#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define INLIMBO_FRONTEND_ABI_VERSION 1

  struct InLimboFrontendAPI
  {
    uint32_t abi_version;
    size_t   struct_size;

    void* (*create)(void* song_map, void* mpris_service);
    void (*run)(void* instance, void* audio_service);
    void (*destroy)(void* instance);
  };

  const struct InLimboFrontendAPI* inlimbo_frontend_get_api(void);

#ifdef __cplusplus
}
#endif
