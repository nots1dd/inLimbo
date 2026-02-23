#include "c/mpris.h"
#include <dbus/dbus.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct mpris_service {
    DBusConnection* conn;
    char bus_name[128];
    mpris_backend backend;

    char title[256];
    char artist[256];
    char album[256];
    char art_url[512];
    char track_id[128];
};

void mpris_update_metadata(
    mpris_service* s,
    const char* title,
    const char* artist,
    const char* album,
    const char* art_url)
{
    if (!s) return;

    snprintf(s->title, sizeof(s->title), "%s", title ? title : "");
    snprintf(s->artist, sizeof(s->artist), "%s", artist ? artist : "");
    snprintf(s->album, sizeof(s->album), "%s", album ? album : "");
    snprintf(s->art_url, sizeof(s->art_url), "%s", art_url ? art_url : "");
    static uint64_t counter = 1;
    snprintf(s->track_id, sizeof(s->track_id),
    "/org/mpris/MediaPlayer2/track/%lu", counter++);
}

static DBusHandlerResult
on_message(DBusConnection* conn, DBusMessage* msg, void* data)
{
    mpris_service* s = data;
    const char* iface = dbus_message_get_interface(msg);
    const char* member = dbus_message_get_member(msg);

    /* Player methods */
    if (iface && !strcmp(iface, "org.mpris.MediaPlayer2.Player")) {

        int should_emit = 0;

        if (!strcmp(member, "Play")) {
            s->backend.play(s->backend.userdata);
            should_emit = 1;
        }
        else if (!strcmp(member, "Pause")) {
            s->backend.pause(s->backend.userdata);
            should_emit = 1;
        }
        else if (!strcmp(member, "Stop")) {
            s->backend.stop(s->backend.userdata);
            should_emit = 1;
        }
        else if (!strcmp(member, "Next")) {
            s->backend.next(s->backend.userdata);
            should_emit = 1;

            if (s->backend.refresh_metadata)
                s->backend.refresh_metadata(s->backend.userdata, s);
        }
        else if (!strcmp(member, "Previous")) {
            s->backend.previous(s->backend.userdata);
            should_emit = 1;

            if (s->backend.refresh_metadata)
                s->backend.refresh_metadata(s->backend.userdata, s);
        }
        else if (!strcmp(member, "Seek")) {
            int64_t off = 0;
            dbus_message_get_args(msg, NULL,
                DBUS_TYPE_INT64, &off,
                DBUS_TYPE_INVALID);

            s->backend.seek(s->backend.userdata, off / 1e6);
            should_emit = 1;
        }
        else if (!strcmp(member, "SetPosition")) {
            const char* path = NULL;
            int64_t pos = 0;

            dbus_message_get_args(msg, NULL,
                DBUS_TYPE_OBJECT_PATH, &path,
                DBUS_TYPE_INT64, &pos,
                DBUS_TYPE_INVALID);

            (void)path; /* we ignore trackid path for now */
            s->backend.set_position(s->backend.userdata, pos / 1e6);
            should_emit = 1;
        }

        if (should_emit)
            mpris_emit(s);

        DBusMessage* reply = dbus_message_new_method_return(msg);
        dbus_connection_send(conn, reply, NULL);
        dbus_message_unref(reply);

        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (dbus_message_is_method_call(
            msg, "org.freedesktop.DBus.Properties", "GetAll")) {

        const char* iface_name;
        dbus_message_get_args(msg, NULL,
            DBUS_TYPE_STRING, &iface_name,
            DBUS_TYPE_INVALID);

        DBusMessage* reply = dbus_message_new_method_return(msg);
        DBusMessageIter it, props_dict;
        dbus_message_iter_init_append(reply, &it);
        dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "{sv}", &props_dict);

        if (!strcmp(iface_name, "org.mpris.MediaPlayer2.Player")) {

            /* Capability flags */
            {
                const char* keys[] = {
                    "CanPlay",
                    "CanPause",
                    "CanSeek",
                    "CanControl",
                    "CanGoNext",
                    "CanGoPrevious"
                };

                dbus_bool_t t = 1;

                for (int i = 0; i < 6; ++i) {
                    DBusMessageIter e, v;
                    dbus_message_iter_open_container(&props_dict, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &keys[i]);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "b", &v);
                    dbus_message_iter_append_basic(&v, DBUS_TYPE_BOOLEAN, &t);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&props_dict, &e);
                }
            }

            /* PlaybackStatus */
            {
                DBusMessageIter e, v;
                const char* key = "PlaybackStatus";
                const char* status =
                    s->backend.is_playing(s->backend.userdata) ? "Playing" : "Paused";

                dbus_message_iter_open_container(&props_dict, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &key);
                dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "s", &v);
                dbus_message_iter_append_basic(&v, DBUS_TYPE_STRING, &status);
                dbus_message_iter_close_container(&e, &v);
                dbus_message_iter_close_container(&props_dict, &e);
            }

            /* Position */
            {
                DBusMessageIter e, v;
                const char* key = "Position";
                int64_t pos = (int64_t)(s->backend.position(s->backend.userdata) * 1e6);

                dbus_message_iter_open_container(&props_dict, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &key);
                dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "x", &v);
                dbus_message_iter_append_basic(&v, DBUS_TYPE_INT64, &pos);
                dbus_message_iter_close_container(&e, &v);
                dbus_message_iter_close_container(&props_dict, &e);
            }

            /* Metadata */
            {
                DBusMessageIter meta_entry, meta_var, meta_array;

                const char* key = "Metadata";
                dbus_message_iter_open_container(&props_dict, DBUS_TYPE_DICT_ENTRY, NULL, &meta_entry);
                dbus_message_iter_append_basic(&meta_entry, DBUS_TYPE_STRING, &key);
                dbus_message_iter_open_container(&meta_entry, DBUS_TYPE_VARIANT, "a{sv}", &meta_var);
                dbus_message_iter_open_container(&meta_var, DBUS_TYPE_ARRAY, "{sv}", &meta_array);

                /* mpris:trackid */
                {
                    DBusMessageIter e, v;
                    const char* k = "mpris:trackid";
                    const char* val = s->track_id;

                    dbus_message_iter_open_container(&meta_array, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "o", &v);
                    dbus_message_iter_append_basic(&v, DBUS_TYPE_OBJECT_PATH, &val);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&meta_array, &e);
                }

                /* xesam:title */
                {
                    DBusMessageIter e, v;
                    const char* k = "xesam:title";
                    const char* title = s->title;
                    dbus_message_iter_open_container(&meta_array, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "s", &v);
                    dbus_message_iter_append_basic(&v, DBUS_TYPE_STRING, &title);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&meta_array, &e);
                }

                /* xesam:artist */
                {
                    DBusMessageIter e, v, arr;
                    const char* k = "xesam:artist";
                    const char* artist = s->artist;
                    dbus_message_iter_open_container(&meta_array, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "as", &v);
                    dbus_message_iter_open_container(&v, DBUS_TYPE_ARRAY, "s", &arr);
                    dbus_message_iter_append_basic(&arr, DBUS_TYPE_STRING, &artist);
                    dbus_message_iter_close_container(&v, &arr);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&meta_array, &e);
                }

                /* xesam:album */
                {
                    DBusMessageIter e, v;
                    const char* k = "xesam:album";
                    const char* album = s->album;
                    dbus_message_iter_open_container(&meta_array, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "s", &v);
                    dbus_message_iter_append_basic(&v, DBUS_TYPE_STRING, &album);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&meta_array, &e);
                }

                /* mpris:length */
                {
                    DBusMessageIter e, v;
                    const char* k = "mpris:length";
                    int64_t len = (int64_t)(s->backend.duration(s->backend.userdata) * 1e6);
                    dbus_message_iter_open_container(&meta_array, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "x", &v);
                    dbus_message_iter_append_basic(&v, DBUS_TYPE_INT64, &len);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&meta_array, &e);
                }

                /* mpris:artUrl */
                {
                    DBusMessageIter e, v;
                    const char* k = "mpris:artUrl";
                    const char* val = s->art_url;

                    dbus_message_iter_open_container(&meta_array, DBUS_TYPE_DICT_ENTRY, NULL, &e);
                    dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
                    dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "s", &v);
                    dbus_message_iter_append_basic(&v, DBUS_TYPE_STRING, &val);
                    dbus_message_iter_close_container(&e, &v);
                    dbus_message_iter_close_container(&meta_array, &e);
                }

                dbus_message_iter_close_container(&meta_var, &meta_array);
                dbus_message_iter_close_container(&meta_entry, &meta_var);
                dbus_message_iter_close_container(&props_dict, &meta_entry);
            }
        }

        dbus_message_iter_close_container(&it, &props_dict);
        dbus_connection_send(conn, reply, NULL);
        dbus_message_unref(reply);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

mpris_service* mpris_create(const char* name, mpris_backend backend)
{
    DBusError err;
    dbus_error_init(&err);

    mpris_service* s = calloc(1, sizeof(*s));
    s->backend = backend;

    snprintf(s->bus_name, sizeof(s->bus_name),
             "org.mpris.MediaPlayer2.%s", name);
    
    snprintf(s->track_id, sizeof(s->track_id),
         "/org/mpris/MediaPlayer2/track/0");

    s->conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (!s->conn) {
        free(s);
        return NULL;
    }

    dbus_bus_request_name(
        s->conn,
        s->bus_name,
        DBUS_NAME_FLAG_DO_NOT_QUEUE,
        &err
    );

    static DBusObjectPathVTable vtable = {
        .message_function = on_message
    };

    dbus_connection_register_object_path(
        s->conn,
        "/org/mpris/MediaPlayer2",
        &vtable,
        s
    );

    return s;
}

void mpris_poll(mpris_service* s)
{
    if (!s || !s->conn) return;

    dbus_connection_read_write(s->conn, 0);
    while (dbus_connection_dispatch(s->conn)
           == DBUS_DISPATCH_DATA_REMAINS);
}

void mpris_emit(mpris_service* s)
{
    if (!s || !s->conn) return;

    DBusMessage* sig = dbus_message_new_signal(
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");

    DBusMessageIter it, changed, invalidated;
    dbus_message_iter_init_append(sig, &it);

    /* Interface name */
    const char* iface = "org.mpris.MediaPlayer2.Player";
    dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &iface);

    /* changed_properties */
    dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "{sv}", &changed);

    {
        DBusMessageIter e, v;
        const char* key = "PlaybackStatus";
        const char* status =
            s->backend.is_playing(s->backend.userdata)
                ? "Playing"
                : "Paused";

        dbus_message_iter_open_container(&changed, DBUS_TYPE_DICT_ENTRY, NULL, &e);
        dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &key);
        dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "s", &v);
        dbus_message_iter_append_basic(&v, DBUS_TYPE_STRING, &status);
        dbus_message_iter_close_container(&e, &v);
        dbus_message_iter_close_container(&changed, &e);
    }

    {
        DBusMessageIter e, v;
        const char* key = "Position";
        int64_t pos =
            (int64_t)(s->backend.position(s->backend.userdata) * 1e6);

        dbus_message_iter_open_container(&changed, DBUS_TYPE_DICT_ENTRY, NULL, &e);
        dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &key);
        dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "x", &v);
        dbus_message_iter_append_basic(&v, DBUS_TYPE_INT64, &pos);
        dbus_message_iter_close_container(&e, &v);
        dbus_message_iter_close_container(&changed, &e);
    }

    {
        DBusMessageIter e, v, dict;

        const char* key = "Metadata";
        dbus_message_iter_open_container(&changed, DBUS_TYPE_DICT_ENTRY, NULL, &e);
        dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &key);
        dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "a{sv}", &v);
        dbus_message_iter_open_container(&v, DBUS_TYPE_ARRAY, "{sv}", &dict);

        /* mpris:trackid */
        {
            DBusMessageIter e, v;
            const char* k = "mpris:trackid";
            const char* val = s->track_id;

            dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &e);
            dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
            dbus_message_iter_open_container(&e, DBUS_TYPE_VARIANT, "o", &v);
            dbus_message_iter_append_basic(&v, DBUS_TYPE_OBJECT_PATH, &val);
            dbus_message_iter_close_container(&e, &v);
            dbus_message_iter_close_container(&dict, &e);
        }

        /* xesam:title */
        {
            DBusMessageIter de, dv;
            const char* k = "xesam:title";
            const char* val = s->title;
            dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &de);
            dbus_message_iter_append_basic(&de, DBUS_TYPE_STRING, &k);
            dbus_message_iter_open_container(&de, DBUS_TYPE_VARIANT, "s", &dv);
            dbus_message_iter_append_basic(&dv, DBUS_TYPE_STRING, &val);
            dbus_message_iter_close_container(&de, &dv);
            dbus_message_iter_close_container(&dict, &de);
        }

        /* xesam:artist (array of strings) */
        {
            DBusMessageIter de, dv, arr;
            const char* k = "xesam:artist";
            const char* val = s->artist;
            dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &de);
            dbus_message_iter_append_basic(&de, DBUS_TYPE_STRING, &k);
            dbus_message_iter_open_container(&de, DBUS_TYPE_VARIANT, "as", &dv);
            dbus_message_iter_open_container(&dv, DBUS_TYPE_ARRAY, "s", &arr);
            dbus_message_iter_append_basic(&arr, DBUS_TYPE_STRING, &val);
            dbus_message_iter_close_container(&dv, &arr);
            dbus_message_iter_close_container(&de, &dv);
            dbus_message_iter_close_container(&dict, &de);
        }

        /* xesam:album */
        {
            DBusMessageIter de, dv;
            const char* k = "xesam:album";
            const char* val = s->album;
            dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &de);
            dbus_message_iter_append_basic(&de, DBUS_TYPE_STRING, &k);
            dbus_message_iter_open_container(&de, DBUS_TYPE_VARIANT, "s", &dv);
            dbus_message_iter_append_basic(&dv, DBUS_TYPE_STRING, &val);
            dbus_message_iter_close_container(&de, &dv);
            dbus_message_iter_close_container(&dict, &de);
        }

        /* mpris:artUrl */
        {
            DBusMessageIter de, dv;
            const char* k = "mpris:artUrl";
            const char* val = s->art_url;

            dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &de);
            dbus_message_iter_append_basic(&de, DBUS_TYPE_STRING, &k);
            dbus_message_iter_open_container(&de, DBUS_TYPE_VARIANT, "s", &dv);
            dbus_message_iter_append_basic(&dv, DBUS_TYPE_STRING, &val);
            dbus_message_iter_close_container(&de, &dv);
            dbus_message_iter_close_container(&dict, &de);
        }

        /* mpris:length (µs) */
        {
            DBusMessageIter de, dv;
            const char* k = "mpris:length";
            int64_t len =
                (int64_t)(s->backend.duration(s->backend.userdata) * 1e6);

            dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY, NULL, &de);
            dbus_message_iter_append_basic(&de, DBUS_TYPE_STRING, &k);
            dbus_message_iter_open_container(&de, DBUS_TYPE_VARIANT, "x", &dv);
            dbus_message_iter_append_basic(&dv, DBUS_TYPE_INT64, &len);
            dbus_message_iter_close_container(&de, &dv);
            dbus_message_iter_close_container(&dict, &de);
        }

        dbus_message_iter_close_container(&v, &dict);
        dbus_message_iter_close_container(&e, &v);
        dbus_message_iter_close_container(&changed, &e);
    }

    dbus_message_iter_close_container(&it, &changed);

    /* invalidated_properties (empty) */
    dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "s", &invalidated);
    dbus_message_iter_close_container(&it, &invalidated);

    dbus_connection_send(s->conn, sig, NULL);
    dbus_message_unref(sig);
}

void mpris_destroy(mpris_service* s)
{
    if (!s) return;
    dbus_connection_unref(s->conn);
    free(s);
}
