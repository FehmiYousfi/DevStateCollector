#include <glibmm.h>
#include <dbus/dbus-glib.h>
#include <iostream>
#include <fstream>
#include "dbus_service.h"

// Function to load introspection XML data
std::string load_introspection_xml(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open introspection XML file: " << file_path << std::endl;
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// Function to handle the "Introspect" call
static char* introspect_callback(DBusGProxy* proxy, GError** error, gpointer user_data) {
    std::string introspection_xml = load_introspection_xml("dbus/CustomTarget-introspection.xml");
    if (introspection_xml.empty()) {
        return g_strdup("<node></node>");
    }
    return g_strdup(introspection_xml.c_str());
}

int main(int argc, char** argv) {
    GError* error = nullptr;
    DBusGConnection* connection;

    // Connect to the system bus
    connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
    if (!connection) {
        std::cerr << "Failed to connect to the D-Bus system bus: " << error->message << std::endl;
        g_error_free(error);
        return 1;
    }

    // Create a new instance of MyDBusService
    MyDBusService* my_service = (MyDBusService*)g_object_new(MY_DBUS_SERVICE_TYPE, nullptr);

    // Register the object on the D-Bus with the new service name
    const char* object_path = "/org/freedesktop/CustomTarget";
    dbus_g_connection_register_g_object(connection, object_path, G_OBJECT(my_service));
    std::cout << "Service registered on D-Bus at " << object_path << std::endl;

    // Register introspection for the service
    DBusGProxy* introspect_proxy = dbus_g_proxy_new_for_name(connection, "org.freedesktop.DBus.Introspectable", object_path, "org.freedesktop.DBus.Introspectable");
    dbus_g_proxy_add_signal(introspect_proxy, "Introspect", G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_object_register_marshaller(g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, G_TYPE_STRING, G_TYPE_INVALID);
    dbus_g_proxy_connect_signal(introspect_proxy, "Introspect", G_CALLBACK(introspect_callback), nullptr, nullptr);

    // Main event loop
    Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create();
    loop->run();

    return 0;
}
