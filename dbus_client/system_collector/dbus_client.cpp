#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <optional>


DBusConnection* connectToBus(DBusError* error) {
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, error);
    if (dbus_error_is_set(error)) {
        std::cerr << "Error connecting to the system bus: " << error->message << std::endl;
        dbus_error_free(error);
    }

    if (!connection) {
        std::cerr << "Failed to connect to the system bus" << std::endl;
        std::exit(1);
    }

    return connection;
}

std::string getUPowerProperty(DBusConnection* connection, const char* propertyName) {
    DBusMessage* msg;
    DBusMessage* reply;
    DBusError error;
    dbus_error_init(&error);

    // Create a method call to org.freedesktop.DBus.Properties.Get
    msg = dbus_message_new_method_call("org.freedesktop.UPower",               // Target for the method call
                                       "/org/freedesktop/UPower",              // Object path
                                       "org.freedesktop.DBus.Properties",      // Interface
                                       "Get");                                 // Method name

    if (!msg) {
        std::cerr << "Failed to create message" << std::endl;
        std::exit(1);
    }

    // Append arguments: interface name and property name
    const char* interfaceName = "org.freedesktop.UPower";
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_STRING, &propertyName, DBUS_TYPE_INVALID);

    // Send the message and wait for a reply
    reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &error);

    std::string propertyValue;
    if (dbus_error_is_set(&error)) {
        std::cerr << "Error in D-Bus method call: " << error.message << std::endl;
        dbus_error_free(&error);
    } else if (reply) {
        DBusMessageIter args;
        if (dbus_message_iter_init(reply, &args)) {
            if (DBUS_TYPE_VARIANT == dbus_message_iter_get_arg_type(&args)) {
                DBusMessageIter variantIter;
                dbus_message_iter_recurse(&args, &variantIter);
                if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&variantIter)) {
                    const char* value;
                    dbus_message_iter_get_basic(&variantIter, &value);
                    propertyValue = value;
                } else if (DBUS_TYPE_BOOLEAN == dbus_message_iter_get_arg_type(&variantIter)) {
                    bool value;
                    dbus_message_iter_get_basic(&variantIter, &value);
                    propertyValue = value ? "true" : "false";
                }
                // Add more types as needed
            }
        }
        dbus_message_unref(reply);
    }

    dbus_message_unref(msg);
    return propertyValue;
}

void requestUPowerProperties(DBusConnection* connection) {
    std::cout << "DaemonVersion: " << getUPowerProperty(connection, "DaemonVersion") << std::endl;
    std::cout << "LidIsClosed: " << getUPowerProperty(connection, "LidIsClosed") << std::endl;
    std::cout << "LidIsPresent: " << getUPowerProperty(connection, "LidIsPresent") << std::endl;
    std::cout << "OnBattery: " << getUPowerProperty(connection, "OnBattery") << std::endl;
}

DBusHandlerResult messageHandler(DBusConnection* connection __attribute__((unused)), 
                                 DBusMessage* message, 
                                 void* user_data __attribute__((unused))) {
    const char* interface = dbus_message_get_interface(message);
    const char* member = dbus_message_get_member(message);

    if (strcmp(interface, "org.freedesktop.UPower") == 0) {
        if (strcmp(member, "DeviceAdded") == 0) {
            std::cout << "Device Added" << std::endl;
        } else if (strcmp(member, "DeviceRemoved") == 0) {
            std::cout << "Device Removed" << std::endl;
        } else if (strcmp(member, "PropertiesChanged") == 0) {
            std::cout << "Properties Changed" << std::endl;
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

void addMessageFilter(DBusConnection* connection) {
    dbus_bus_add_match(connection, 
        "type='signal',interface='org.freedesktop.UPower'", 
        nullptr);
    dbus_connection_add_filter(connection, messageHandler, nullptr, nullptr);
}

void runMainLoop(DBusConnection* connection) {
    while (true) {
        dbus_connection_read_write(connection, 0);
        DBusMessage* message = dbus_connection_pop_message(connection);

        if (message) {
            dbus_message_unref(message);
        }
    }
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = connectToBus(&error);

    // Request UPower properties at startup
    requestUPowerProperties(connection);

    addMessageFilter(connection);
    runMainLoop(connection);

    dbus_connection_unref(connection);
    return 0;
}
