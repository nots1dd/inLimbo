/**
 * @file MPRISServer.hpp
 * @brief MPRISService class definition and implementation for interacting with MPRIS over D-Bus.
 *
 * The `MPRISService` class implements a service for interacting with the MPRIS (Media Player Remote
 * Interfacing Specification) over D-Bus. It exposes player properties and methods as defined by the
 * MPRIS standard, allowing other applications to interact with a media player (e.g., controlling
 * playback, getting metadata).
 *
 * The service provides methods to update metadata, handle playback control, and register the
 * service on the D-Bus.
 */

#ifndef MPRIS_SERVER_HPP
#define MPRIS_SERVER_HPP

#include <gio/gio.h>
#include <glib.h>
#include <iostream>
#include <memory>
#include <string>

/**
 * @class MPRISService
 * @brief A class that implements an MPRIS service over D-Bus.
 *
 * This class provides a service for exposing media player properties and methods over D-Bus
 * using the MPRIS interface, which includes features like controlling playback, retrieving
 * metadata, and exposing basic media player information such as supported URI schemes.
 */

class MPRISService
{
public:
  /**
   * @brief Constructs a new MPRISService object.
   *
   * This constructor sets up the D-Bus connection, registers the service on the session bus,
   * and initializes default metadata values for the media player.
   *
   * @param applicationName The name of the media player application.
   */
  MPRISService(const std::string& applicationName) : applicationName_(applicationName)
  {

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

    GError* error       = nullptr;
    introspection_data_ = g_dbus_node_info_new_for_xml(introspection_xml, &error);
    if (error != nullptr)
    {
      std::cerr << "Error creating introspection data: " << error->message << std::endl;
      g_error_free(error);
      return;
    }

    connection_ = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (error != nullptr)
    {
      std::cerr << "Error connecting to D-Bus: " << error->message << std::endl;
      g_error_free(error);
      return;
    }

    // Register both interfaces
    const GDBusInterfaceVTable root_vtable = {
      handle_root_method_call, handle_root_get_property, nullptr, {nullptr}};

    const GDBusInterfaceVTable player_vtable = {
      handle_player_method_call, handle_player_get_property, handle_player_set_property, {nullptr}};

    std::string busName = "org.mpris.MediaPlayer2." + applicationName_;
    guint       owner_id =
      g_bus_own_name(G_BUS_TYPE_SESSION, busName.c_str(), G_BUS_NAME_OWNER_FLAGS_REPLACE,
                     on_bus_acquired, nullptr, nullptr, this, nullptr);

    // Register both interfaces
    g_dbus_connection_register_object(connection_, "/org/mpris/MediaPlayer2",
                                      introspection_data_->interfaces[0], // Root interface
                                      &root_vtable, this, nullptr, &error);

    g_dbus_connection_register_object(connection_, "/org/mpris/MediaPlayer2",
                                      introspection_data_->interfaces[1], // Player interface
                                      &player_vtable, this, nullptr, &error);

    if (error != nullptr)
    {
      std::cerr << "Error registering object: " << error->message << std::endl;
      g_error_free(error);
    }

    // Initialize default metadata
    updateMetadata("Unknown Song", "Unknown Artist", "Unknown Album", 0, "No Comment", "No genre",
                   0, 0);
  }
  /**
   * @brief Destroys the MPRISService object, releasing resources.
   *
   * Cleans up allocated resources including D-Bus connection, introspection data, and metadata.
   */

  ~MPRISService()
  {
    if (connection_)
    {
      g_object_unref(connection_);
      connection_ = nullptr;
    }
    if (introspection_data_)
    {
      g_dbus_node_info_unref(introspection_data_);
      introspection_data_ = nullptr;
    }
    if (current_metadata_)
    {
      g_variant_unref(current_metadata_);
      current_metadata_ = nullptr;
    }
    std::cout << "-- MPRISService cleaned up." << std::endl;
  }

  /**
   * @brief Updates the media player metadata.
   *
   * This method allows updating the metadata associated with the media player, including track
   * title, artist, album, length, comment, genre, track number, and disc number. The metadata is
   * emitted via the D-Bus signal to notify listeners.
   *
   * @param title The track title.
   * @param artist The artist name.
   * @param album The album name.
   * @param length The track length in seconds.
   * @param comment (Optional) Additional comments for the track.
   * @param genre (Optional) Genre of the track.
   * @param trackNumber (Optional) The track number in the album.
   * @param discNumber (Optional) The disc number in the album.
   */

  void updateMetadata(const std::string& title, const std::string& artist, const std::string& album,
                      int64_t length, const std::string& comment = "",
                      const std::string& genre = "", int trackNumber = 0, int discNumber = 0)
  {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    // Required field
    std::string trackid = "/org/mpris/MediaPlayer2/Track/" + std::to_string(rand());
    g_variant_builder_add(&builder, "{sv}", "mpris:trackid",
                          g_variant_new_object_path(trackid.c_str()));

    g_variant_builder_add(&builder, "{sv}", "xesam:title", g_variant_new_string(title.c_str()));

    GVariantBuilder artist_builder;
    g_variant_builder_init(&artist_builder, G_VARIANT_TYPE("as"));
    g_variant_builder_add(&artist_builder, "s", artist.c_str());
    g_variant_builder_add(&builder, "{sv}", "xesam:artist", g_variant_builder_end(&artist_builder));

    g_variant_builder_add(&builder, "{sv}", "xesam:album", g_variant_new_string(album.c_str()));

    g_variant_builder_add(&builder, "{sv}", "mpris:length", g_variant_new_int64(length * 1000000));

    // Add optional fields
    if (!comment.empty())
    {
      g_variant_builder_add(&builder, "{sv}", "xesam:comment",
                            g_variant_new_string(comment.c_str()));
    }
    if (!genre.empty())
    {
      GVariantBuilder genre_builder;
      g_variant_builder_init(&genre_builder, G_VARIANT_TYPE("as"));
      g_variant_builder_add(&genre_builder, "s", genre.c_str());
      g_variant_builder_add(&builder, "{sv}", "xesam:genre", g_variant_builder_end(&genre_builder));
    }
    if (trackNumber > 0)
    {
      g_variant_builder_add(&builder, "{sv}", "xesam:trackNumber",
                            g_variant_new_int32(trackNumber));
    }
    if (discNumber > 0)
    {
      g_variant_builder_add(&builder, "{sv}", "xesam:discNumber", g_variant_new_int32(discNumber));
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
      connection_, nullptr, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties",
      "PropertiesChanged",
      g_variant_new("(sa{sv}as)", "org.mpris.MediaPlayer2.Player", &props_builder, nullptr),
      nullptr);
  }

private:
  /**
   * @brief Callback function when the D-Bus bus name is acquired.
   *
   * This function is called when the D-Bus name is successfully acquired, indicating that
   * the service is ready to respond to D-Bus method calls.
   *
   * @param connection The D-Bus connection.
   * @param name The name acquired on the bus.
   * @param user_data User data passed from the service constructor.
   */
  static void on_bus_acquired(GDBusConnection* connection, const gchar* name, gpointer user_data)
  {
    std::cout << "Acquired bus name: " << name << std::endl;
  }

  /**
   * @brief Callback function for handling root interface method calls.
   *
   * This function is invoked when a method from the root interface is called.
   * For now, it returns an empty response as no specific methods are implemented.
   *
   * @param connection The D-Bus connection.
   * @param sender The sender of the method call.
   * @param object_path The object path for the method call.
   * @param interface_name The interface name for the method call.
   * @param method_name The method being invoked.
   * @param parameters The parameters of the method call.
   * @param invocation The invocation object used to return a response.
   * @param user_data User data passed from the service constructor.
   */
  static void handle_root_method_call(GDBusConnection*, const char*, const char*, const char*,
                                      const char*, GVariant*, GDBusMethodInvocation* invocation,
                                      void*)
  {
    g_dbus_method_invocation_return_value(invocation, nullptr);
  }

  /**
   * @brief Callback function for handling player interface method calls.
   *
   * This function handles various method calls (e.g., Play, Pause, Next) on the player interface.
   *
   * @param connection The D-Bus connection.
   * @param sender The sender of the method call.
   * @param object_path The object path for the method call.
   * @param interface_name The interface name for the method call.
   * @param method_name The method being invoked.
   * @param parameters The parameters of the method call.
   * @param invocation The invocation object used to return a response.
   * @param user_data User data passed from the service constructor.
   */
  static void handle_player_method_call(GDBusConnection*, const char*, const char*, const char*,
                                        const char*            method_name, GVariant*,
                                        GDBusMethodInvocation* invocation, void* user_data)
  {
    auto* service = static_cast<MPRISService*>(user_data);
    g_dbus_method_invocation_return_value(invocation, nullptr);
  }

  /**
   * @brief Callback function to handle getting a property from the root interface.
   *
   * This function handles retrieving property values like "Identity", "CanQuit", and "CanRaise".
   *
   * @param connection The D-Bus connection.
   * @param sender The sender of the property request.
   * @param object_path The object path for the property request.
   * @param interface_name The interface name for the property request.
   * @param property_name The name of the property being requested.
   * @param error Error output in case of failure.
   * @param user_data User data passed from the service constructor.
   * @return The value of the requested property.
   */
  static GVariant* handle_root_get_property(GDBusConnection* connection, const char* sender,
                                            const char* object_path, const char* interface_name,
                                            const char* property_name, GError** error,
                                            void* user_data)
  {
    auto* service = static_cast<MPRISService*>(user_data);

    if (g_strcmp0(property_name, "Identity") == 0)
      return g_variant_new_string(service->applicationName_.c_str());
    if (g_strcmp0(property_name, "CanQuit") == 0)
      return g_variant_new_boolean(TRUE);
    if (g_strcmp0(property_name, "CanRaise") == 0)
      return g_variant_new_boolean(TRUE);
    if (g_strcmp0(property_name, "HasTrackList") == 0)
      return g_variant_new_boolean(FALSE);

    // Property not recognized: set error and return nullptr
    g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,
                "Property '%s' is not recognized for interface '%s'.", property_name,
                interface_name);
    return nullptr;
  }

  /**
   * @brief Callback function to handle getting a property from the player interface.
   *
   * This function handles retrieving player properties like "PlaybackStatus", "LoopStatus", and
   * "Volume".
   *
   * @param connection The D-Bus connection.
   * @param sender The sender of the property request.
   * @param object_path The object path for the property request.
   * @param interface_name The interface name for the property request.
   * @param property_name The name of the property being requested.
   * @param error Error output in case of failure.
   * @param user_data User data passed from the service constructor.
   * @return The value of the requested property.
   */
  static GVariant* handle_player_get_property(GDBusConnection* connection, const char* sender,
                                              const char* object_path, const char* interface_name,
                                              const char* property_name, GError** error,
                                              void* user_data)
  {
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

    // Property not recognized: set error and return nullptr
    g_set_error(error, G_IO_ERROR, G_IO_ERROR_FAILED,
                "Property '%s' is not recognized for interface '%s'.", property_name,
                interface_name);
    return nullptr;
  }
  /**
   * @brief Callback function to handle setting a property on the player interface.
   *
   * This function handles setting player properties such as volume, playback status, etc.
   *
   * @param connection The D-Bus connection.
   * @param sender The sender of the property request.
   * @param object_path The object path for the property request.
   * @param interface_name The interface name for the property request.
   * @param property_name The name of the property being set.
   * @param value The new value of the property.
   * @param error Error output in case of failure.
   * @param user_data User data passed from the service constructor.
   */
  static gboolean handle_player_set_property(GDBusConnection*, const char*, const char*,
                                             const char*, const char*, GVariant*, GError**, void*)
  {
    return TRUE;
  }

  std::string      applicationName_;
  GDBusNodeInfo*   introspection_data_ = nullptr;
  GDBusConnection* connection_         = nullptr;
  GVariant*        current_metadata_   = nullptr;
};

#endif
