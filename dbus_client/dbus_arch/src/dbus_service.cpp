#include "dbus_service.h"
#include <gio/gio.h>
#include <iostream>

// Define the GObject type for MyDBusService
G_DEFINE_TYPE(MyDBusService, my_dbus_service, G_TYPE_OBJECT);

// Method to handle D-Bus method call
static gboolean say_hello(MyDBusService* self, gchar** message, GError** error) {
    *message = g_strdup("Hello from the D-Bus service!");
    std::cout << "SayHello method called: Returning 'Hello from the D-Bus service!'" << std::endl;
    return TRUE;
}

// Callback for handling D-Bus method calls
static gboolean on_handle_method(GDBusConnection* connection, const gchar* sender, const gchar* object_path,
                                 const gchar* interface_name, const gchar* method_name, GVariant* parameters,
                                 GVariant** reply, GError** error, gpointer user_data) {
    MyDBusService* service = MY_DBUS_SERVICE(user_data);
    gchar* message = nullptr;
    gboolean result = say_hello(service, &message, error);
    if (result) {
        *reply = g_variant_new("(s)", message);
    }
    g_free(message);
    return result;
}

static void my_dbus_service_init(MyDBusService* self) {
    // Initialization code for MyDBusService if needed
}

static void my_dbus_service_class_init(MyDBusServiceClass* klass) {
    // Class initialization code for MyDBusService if needed
}
