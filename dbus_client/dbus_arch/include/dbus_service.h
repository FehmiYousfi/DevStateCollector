#ifndef DBUS_SERVICE_H
#define DBUS_SERVICE_H

#include <glib-object.h>

// Define the GObject type for MyDBusService
#define MY_DBUS_SERVICE_TYPE (my_dbus_service_get_type())
#define MY_DBUS_SERVICE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MY_DBUS_SERVICE_TYPE, MyDBusService))

typedef struct _MyDBusService MyDBusService;
typedef struct _MyDBusServiceClass MyDBusServiceClass;

struct _MyDBusService {
    GObject parent_instance;
};

struct _MyDBusServiceClass {
    GObjectClass parent_class;
};

GType my_dbus_service_get_type(void);

#endif // DBUS_SERVICE_H
