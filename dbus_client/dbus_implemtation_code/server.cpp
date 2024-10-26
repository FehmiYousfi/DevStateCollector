#include <glibmm.h>
#include <dbus/dbus-glib.h>
#include <iostream>

#define MY_DBUS_SERVICE_TYPE my_dbus_service_get_type()
#define MY_DBUS_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MY_DBUS_SERVICE_TYPE, MyDBusService))

// Define a GObject class for MyDBusService
typedef struct _MyDBusService {
    GObject parent_instance;
} MyDBusService;

typedef struct _MyDBusServiceClass {
    GObjectClass parent_class;
} MyDBusServiceClass;

G_DEFINE_TYPE(MyDBusService, my_dbus_service, G_TYPE_OBJECT);

// Method to handle D-Bus method call
gboolean my_method_handler(MyDBusService* self, gchar** result, GError** error) {
    *result = g_strdup("Hello from the D-Bus service!");
    std::cout << "Method called: Returning 'Hello from the D-Bus service!'" << std::endl;
    return TRUE;
}

// Callback for handling D-Bus method calls
static gboolean on_handle_method(MyDBusService* self, gchar** result, GError** error, gpointer user_data) {
    return my_method_handler(self, result, error);
}

static void my_dbus_service_init(MyDBusService* self) {
    // Initialization code for MyDBusService if needed
}

static void my_dbus_service_class_init(MyDBusServiceClass* klass) {
    // Class initialization code for MyDBusService if needed
}

int main(int argc, char** argv) {
    GError* error = nullptr;
    DBusGConnection* connection;

    // Connect to the session bus
    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (!connection) {
        std::cerr << "Failed to connect to the D-Bus session bus: " << error->message << std::endl;
        g_error_free(error);
        return 1;
    }

    // Create a new instance of MyDBusService using g_object_new
    MyDBusService* my_service = (MyDBusService*)g_object_new(MY_DBUS_SERVICE_TYPE, nullptr);

    // Register the service name on the D-Bus
    const char* service_name = "org.example.MyService";
    const char* object_path = "/org/example/MyService";

    // Register the object on the D-Bus
    dbus_g_connection_register_g_object(connection, object_path, G_OBJECT(my_service));
    std::cout << "Service registered on D-Bus: " << service_name << " at " << object_path << std::endl;

    // Main event loop
    Glib::RefPtr<Glib::MainLoop> loop = Glib::MainLoop::create();
    loop->run();

    return 0;
}
