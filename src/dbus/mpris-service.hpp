#ifndef MPRIS_SERVER_HPP
#define MPRIS_SERVER_HPP

#include <gio/gio.h>
#include <glib.h>
#include <string>
#include <memory>
#include <iostream>

class MPRISService {
public:
    MPRISService(const std::string& applicationName) 
        : applicationName_(applicationName) {
        
        const char* introspection_xml = R"XML(
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<node>
    <interface name="org.mpris.MediaPlayer2">
        <property name="Identity" type="s" access="read"/>
        <property name="DesktopEntry" type="s" access="read"/>
        <property name="SupportedUriSchemes" type="as" access="read"/>
        <property name="SupportedMimeTypes" type="as" access="read"/>
        <property name="CanQuit" type="b" access="read"/>
        <property name="CanRaise" type="b" access="read"/>
        <property name="HasTrackList" type="b" access="read"/>
    </interface>
    <interface name="org.mpris.MediaPlayer2.Player">
        <property name="PlaybackStatus" type="s" access="read"/>
        <property name="LoopStatus" type="s" access="readwrite"/>
        <property name="Rate" type="d" access="readwrite"/>
        <property name="Shuffle" type="b" access="readwrite"/>
        <property name="Metadata" type="a{sv}" access="read"/>
        <property name="Volume" type="d" access="readwrite"/>
        <property name="Position" type="x" access="read"/>
        <property name="MinimumRate" type="d" access="read"/>
        <property name="MaximumRate" type="d" access="read"/>
        <property name="CanGoNext" type="b" access="read"/>
        <property name="CanGoPrevious" type="b" access="read"/>
        <property name="CanPlay" type="b" access="read"/>
        <property name="CanPause" type="b" access="read"/>
        <property name="CanSeek" type="b" access="read"/>
        <property name="CanControl" type="b" access="read"/>
        <method name="Next"/>
        <method name="Previous"/>
        <method name="Pause"/>
        <method name="PlayPause"/>
        <method name="Stop"/>
        <method name="Play"/>
        <method name="Seek">
            <arg name="Offset" type="x" direction="in"/>
        </method>
        <method name="SetPosition">
            <arg name="TrackId" type="o" direction="in"/>
            <arg name="Position" type="x" direction="in"/>
        </method>
        <method name="OpenUri">
            <arg name="Uri" type="s" direction="in"/>
        </method>
    </interface>
</node>)XML";

        GError* error = nullptr;
        introspection_data_ = g_dbus_node_info_new_for_xml(introspection_xml, &error);
        if (error != nullptr) {
            std::cerr << "Error creating introspection data: " << error->message << std::endl;
            g_error_free(error);
            return;
        }

        connection_ = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
        if (error != nullptr) {
            std::cerr << "Error connecting to D-Bus: " << error->message << std::endl;
            g_error_free(error);
            return;
        }

        // Register both interfaces
        const GDBusInterfaceVTable root_vtable = {
            handle_root_method_call,
            handle_root_get_property,
            nullptr,
            { nullptr }
        };

        const GDBusInterfaceVTable player_vtable = {
            handle_player_method_call,
            handle_player_get_property,
            handle_player_set_property,
            { nullptr }
        };

        std::string busName = "org.mpris.MediaPlayer2." + applicationName_;
        guint owner_id = g_bus_own_name(
            G_BUS_TYPE_SESSION,
            busName.c_str(),
            G_BUS_NAME_OWNER_FLAGS_REPLACE,
            on_bus_acquired,
            nullptr,
            nullptr,
            this,
            nullptr
        );

        // Register both interfaces
        g_dbus_connection_register_object(
            connection_,
            "/org/mpris/MediaPlayer2",
            introspection_data_->interfaces[0],  // Root interface
            &root_vtable,
            this,
            nullptr,
            &error
        );

        g_dbus_connection_register_object(
            connection_,
            "/org/mpris/MediaPlayer2",
            introspection_data_->interfaces[1],  // Player interface
            &player_vtable,
            this,
            nullptr,
            &error
        );

        if (error != nullptr) {
            std::cerr << "Error registering object: " << error->message << std::endl;
            g_error_free(error);
        }

        // Initialize default metadata
        updateMetadata("Unknown Song", "Unknown Artist", "Unknown Album", 0, "No Comment", "No genre", 0, 0);
    }

    ~MPRISService() { 
        if (connection_) {
            g_object_unref(connection_);
        }
        if (introspection_data_) {
            g_dbus_node_info_unref(introspection_data_);
        }
        if (current_metadata_) {
            g_variant_unref(current_metadata_);
        }
        std::cout << "MPRISService cleaned up." << std::endl;
    }

  void updateMetadata(const std::string& title, 
                   const std::string& artist, 
                   const std::string& album,
                   int64_t length,
                   const std::string& comment = "",
                   const std::string& genre = "",
                   int trackNumber = 0,
                   int discNumber = 0) {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    // Required field
    std::string trackid = "/org/mpris/MediaPlayer2/Track/" + std::to_string(rand());
    g_variant_builder_add(&builder, "{sv}", "mpris:trackid", 
        g_variant_new_object_path(trackid.c_str()));

    g_variant_builder_add(&builder, "{sv}", "xesam:title",
        g_variant_new_string(title.c_str()));

    GVariantBuilder artist_builder;
    g_variant_builder_init(&artist_builder, G_VARIANT_TYPE("as"));
    g_variant_builder_add(&artist_builder, "s", artist.c_str());
    g_variant_builder_add(&builder, "{sv}", "xesam:artist",
        g_variant_builder_end(&artist_builder));

    g_variant_builder_add(&builder, "{sv}", "xesam:album",
        g_variant_new_string(album.c_str()));

    g_variant_builder_add(&builder, "{sv}", "mpris:length",
        g_variant_new_int64(length * 1000000));

    // Add optional fields
    if (!comment.empty()) {
        g_variant_builder_add(&builder, "{sv}", "xesam:comment",
            g_variant_new_string(comment.c_str()));
    }
    if (!genre.empty()) {
        GVariantBuilder genre_builder;
        g_variant_builder_init(&genre_builder, G_VARIANT_TYPE("as"));
        g_variant_builder_add(&genre_builder, "s", genre.c_str());
        g_variant_builder_add(&builder, "{sv}", "xesam:genre",
            g_variant_builder_end(&genre_builder));
    }
    if (trackNumber > 0) {
        g_variant_builder_add(&builder, "{sv}", "xesam:trackNumber",
            g_variant_new_int32(trackNumber));
    }
    if (discNumber > 0) {
        g_variant_builder_add(&builder, "{sv}", "xesam:discNumber",
            g_variant_new_int32(discNumber));
    }

    // Save current metadata
    if (current_metadata_)
        g_variant_unref(current_metadata_);
    current_metadata_ = g_variant_builder_end(&builder);
    g_variant_ref(current_metadata_);

    // Emit property changed
    GVariantBuilder props_builder;
    g_variant_builder_init(&props_builder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add(&props_builder, "{sv}", "Metadata", current_metadata_);

    g_dbus_connection_emit_signal(
        connection_,
        nullptr,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        g_variant_new("(sa{sv}as)",
                     "org.mpris.MediaPlayer2.Player",
                     &props_builder,
                     nullptr),
        nullptr
    );
}

private:
    static void on_bus_acquired(GDBusConnection* connection,
                              const gchar* name,
                              gpointer user_data) {
        std::cout << "Acquired bus name: " << name << std::endl;
    }

    static void handle_root_method_call(GDBusConnection*, const char*, const char*,
                                      const char*, const char*, GVariant*,
                                      GDBusMethodInvocation* invocation, void*) {
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }

    static void handle_player_method_call(GDBusConnection*, const char*, const char*,
                                        const char*, const char* method_name, GVariant*,
                                        GDBusMethodInvocation* invocation, void* user_data) {
        auto* service = static_cast<MPRISService*>(user_data);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }

    static GVariant* handle_root_get_property(GDBusConnection*, const char*, const char*,
                                            const char*, const char* property_name,
                                            GError**, void* user_data) {
        auto* service = static_cast<MPRISService*>(user_data);
        
        if (g_strcmp0(property_name, "Identity") == 0)
            return g_variant_new_string(service->applicationName_.c_str());
        if (g_strcmp0(property_name, "CanQuit") == 0)
            return g_variant_new_boolean(TRUE);
        if (g_strcmp0(property_name, "CanRaise") == 0)
            return g_variant_new_boolean(TRUE);
        if (g_strcmp0(property_name, "HasTrackList") == 0)
            return g_variant_new_boolean(FALSE);
        
        return nullptr;
    }

    static GVariant* handle_player_get_property(GDBusConnection*, const char*, const char*,
                                              const char*, const char* property_name,
                                              GError**, void* user_data) {
        auto* service = static_cast<MPRISService*>(user_data);
        
        if (g_strcmp0(property_name, "PlaybackStatus") == 0)
            return g_variant_new_string("Playing");
        if (g_strcmp0(property_name, "LoopStatus") == 0)
            return g_variant_new_string("None");
        if (g_strcmp0(property_name, "Rate") == 0)
            return g_variant_new_double(1.0);
        if (g_strcmp0(property_name, "Shuffle") == 0)
            return g_variant_new_boolean(FALSE);
        if (g_strcmp0(property_name, "Metadata") == 0)
            return service->current_metadata_ ? g_variant_ref(service->current_metadata_) 
                                            : g_variant_new("a{sv}", nullptr);
        if (g_strcmp0(property_name, "Volume") == 0)
            return g_variant_new_double(1.0);
        if (g_strcmp0(property_name, "Position") == 0)
            return g_variant_new_int64(0);
        if (g_strcmp0(property_name, "MinimumRate") == 0)
            return g_variant_new_double(1.0);
        if (g_strcmp0(property_name, "MaximumRate") == 0)
            return g_variant_new_double(1.0);
        if (g_strcmp0(property_name, "CanGoNext") == 0)
            return g_variant_new_boolean(TRUE);
        if (g_strcmp0(property_name, "CanGoPrevious") == 0)
            return g_variant_new_boolean(TRUE);
        if (g_strcmp0(property_name, "CanPlay") == 0)
            return g_variant_new_boolean(TRUE);
        if (g_strcmp0(property_name, "CanPause") == 0)
            return g_variant_new_boolean(TRUE);
        if (g_strcmp0(property_name, "CanSeek") == 0)
            return g_variant_new_boolean(FALSE);
        if (g_strcmp0(property_name, "CanControl") == 0)
            return g_variant_new_boolean(TRUE);
        
        return nullptr;
    }

    static gboolean handle_player_set_property(GDBusConnection*, const char*, const char*,
                                             const char*, const char*, GVariant*,
                                             GError**, void*) {
        return TRUE;
    }

    std::string applicationName_;
    GDBusNodeInfo* introspection_data_ = nullptr;
    GDBusConnection* connection_ = nullptr;
    GVariant* current_metadata_ = nullptr;
};

#endif
