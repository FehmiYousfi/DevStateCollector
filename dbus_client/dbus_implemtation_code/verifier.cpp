#include <dbus/dbus-glib.h>
#include <iostream>

int main() {
    GError* error = nullptr;
    DBusGConnection* connection;

    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (!connection) {
        std::cerr << "Failed to connect to the D-Bus session bus: " << error->message << std::endl;
        g_error_free(error);
        return 1;
    }

    // Check for the presence of the service
    DBusGProxy* proxy = dbus_g_proxy_new_for_name(connection,
                                                 "org.example.MyService",
                                                 "/org/example/MyService",
                                                 "org.freedesktop.DBus.Introspectable");
    if (proxy) {
        std::cout << "Service 'org.example.MyService' is available at '/org/example/MyService'." << std::endl;
        g_object_unref(proxy);
    } else {
        std::cerr << "Service 'org.example.MyService' is not available." << std::endl;
    }

    return 0;
}
