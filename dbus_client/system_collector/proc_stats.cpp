#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <cstring>

class SystemProcess {
public:
    SystemProcess(DBusConnection* connection);

    void listProcesses();
    void getProcessData(const std::string& processId);
    void printProcessData() const;
    
    // New public method to access process data
    const std::map<std::string, std::string>& getProcesses() const;

private:
    DBusConnection* connection_;
    std::map<std::string, std::string> processes_;
};

// Implementation of new accessor method
const std::map<std::string, std::string>& SystemProcess::getProcesses() const {
    return processes_;
}


SystemProcess::SystemProcess(DBusConnection* connection) : connection_(connection) {}

void SystemProcess::listProcesses() {
    DBusMessage* msg;
    DBusMessage* reply;
    DBusError error;
    dbus_error_init(&error);

    msg = dbus_message_new_method_call("org.freedesktop.DBus",
                                       "/org/freedesktop/DBus",
                                       "org.freedesktop.DBus",
                                       "ListNames");

    if (!msg) {
        std::cerr << "Failed to create message" << std::endl;
        std::exit(1);
    }

    reply = dbus_connection_send_with_reply_and_block(connection_, msg, -1, &error);

    if (dbus_error_is_set(&error)) {
        std::cerr << "Error in D-Bus method call: " << error.message << std::endl;
        dbus_error_free(&error);
    } else if (reply) {
        DBusMessageIter args;
        if (dbus_message_iter_init(reply, &args) && DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&args)) {
            DBusMessageIter arrayIter;
            dbus_message_iter_recurse(&args, &arrayIter);
            while (dbus_message_iter_get_arg_type(&arrayIter) == DBUS_TYPE_STRING) {
                const char* name;
                dbus_message_iter_get_basic(&arrayIter, &name);
                processes_[name] = "No data"; // Placeholder for actual process data
                dbus_message_iter_next(&arrayIter);
            }
        }
        dbus_message_unref(reply);
    }

    dbus_message_unref(msg);
}

void SystemProcess::getProcessData(const std::string& processId) {
    DBusMessage* msg;
    DBusMessage* reply;
    DBusError error;
    dbus_error_init(&error);

    msg = dbus_message_new_method_call("org.freedesktop.DBus",
                                       processId.c_str(), // Placeholder, adjust as needed
                                       "org.freedesktop.DBus",
                                       "GetData"); // Placeholder method name

    if (!msg) {
        std::cerr << "Failed to create message" << std::endl;
        std::exit(1);
    }

    reply = dbus_connection_send_with_reply_and_block(connection_, msg, -1, &error);

    if (dbus_error_is_set(&error)) {
        std::cerr << "Error in D-Bus method call: " << error.message << std::endl;
        dbus_error_free(&error);
    } else if (reply) {
        DBusMessageIter args;
        if (dbus_message_iter_init(reply, &args) && DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
            const char* data;
            dbus_message_iter_get_basic(&args, &data);
            processes_[processId] = data;
        }
        dbus_message_unref(reply);
    }

    dbus_message_unref(msg);
}

void SystemProcess::printProcessData() const {
    for (const auto& [processId, data] : processes_) {
        std::cout << "Process ID: " << processId << " Data: " << data << std::endl;
    }
}

DBusConnection* connectToBus(DBusError* error) {
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SESSION, error);
    if (dbus_error_is_set(error)) {
        std::cerr << "Error connecting to the session bus: " << error->message << std::endl;
        dbus_error_free(error);
    }

    if (!connection) {
        std::cerr << "Failed to connect to the session bus" << std::endl;
        std::exit(1);
    }

    return connection;
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = connectToBus(&error);

    SystemProcess sysProc(connection);
    sysProc.listProcesses();

    // Simulate fetching data for each process
    for (const auto& [processId, _] : sysProc.getProcesses()) {
        sysProc.getProcessData(processId);
    }

    sysProc.printProcessData();

    dbus_connection_unref(connection);
    return 0;
}
