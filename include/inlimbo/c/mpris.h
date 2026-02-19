#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    MPRIS_LOOP_NONE,
    MPRIS_LOOP_TRACK,
    MPRIS_LOOP_PLAYLIST
  } mpris_loop_mode;

  /* Opaque handle */
  typedef struct mpris_service mpris_service;

  /* Backend vtable */
  typedef struct
  {
    void* userdata;

    void (*play)(void*);
    void (*pause)(void*);
    void (*stop)(void*);
    void (*next)(void*);
    void (*previous)(void*);

    void (*seek)(void*, double offset_sec);
    void (*set_position)(void*, double pos_sec);

    bool (*is_playing)(void*);
    double (*position)(void*);
    double (*duration)(void*);

    mpris_loop_mode (*loop_mode)(void*);
    void (*set_loop_mode)(void*, mpris_loop_mode);

    bool (*shuffle)(void*);
    void (*set_shuffle)(void*, bool);

    const char* (*title)(void*);
    const char* (*artist)(void*);
    const char* (*album)(void*);
    const char* (*art_url)(void*);

    void (*refresh_metadata)(void* userdata, struct mpris_service* svc);

  } mpris_backend;

  /* API */
  void           mpris_update_metadata(mpris_service* s, const char* title, const char* artist,
                                       const char* album, const char* art_url);
  mpris_service* mpris_create(const char* name, mpris_backend backend);
  void           mpris_poll(mpris_service* svc);
  void           mpris_emit(mpris_service* svc);
  void           mpris_destroy(mpris_service* svc);

#ifdef __cplusplus
}
#endif
