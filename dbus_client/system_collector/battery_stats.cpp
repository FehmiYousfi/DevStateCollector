#include <dbus/dbus.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <map>

class UPowerDevice {
public:
    UPowerDevice(DBusConnection* connection, const std::string& devicePath);

    void requestProperties();
    void printProperties() const;
    void updateProperty(const std::string& propertyName, const std::string& value);

private:
    std::string getProperty(const std::string& propertyName);
    DBusConnection* connection_;
    std::string devicePath_;
    std::map<std::string, std::string> properties_;
};

UPowerDevice::UPowerDevice(DBusConnection* connection, const std::string& devicePath)
    : connection_(connection), devicePath_(devicePath) {}

std::string UPowerDevice::getProperty(const std::string& propertyName) {
    DBusMessage* msg;
    DBusMessage* reply;
    DBusError error;
    dbus_error_init(&error);

    msg = dbus_message_new_method_call("org.freedesktop.UPower",               
                                       devicePath_.c_str(),                    
                                       "org.freedesktop.DBus.Properties",      
                                       "Get");                                 

    if (!msg) {
        std::cerr << "Failed to create message" << std::endl;
        std::exit(1);
    }

    const char* interfaceName = "org.freedesktop.UPower.Device";
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_STRING, &propertyName, DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(connection_, msg, -1, &error);

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
                int argType = dbus_message_iter_get_arg_type(&variantIter);
                if (argType == DBUS_TYPE_STRING) {
                    const char* value;
                    dbus_message_iter_get_basic(&variantIter, &value);
                    propertyValue = value;
                } else if (argType == DBUS_TYPE_BOOLEAN) {
                    bool value;
                    dbus_message_iter_get_basic(&variantIter, &value);
                    propertyValue = value ? "true" : "false";
                } else if (argType == DBUS_TYPE_UINT32) {
                    uint32_t value;
                    dbus_message_iter_get_basic(&variantIter, &value);
                    propertyValue = std::to_string(value);
                } else if (argType == DBUS_TYPE_DOUBLE) {
                    double value;
                    dbus_message_iter_get_basic(&variantIter, &value);
                    propertyValue = std::to_string(value);
                }
                // Add more types as needed
            }
        }
        dbus_message_unref(reply);
    }

    dbus_message_unref(msg);
    return propertyValue;
}

void UPowerDevice::updateProperty(const std::string& propertyName, const std::string& value) {
    properties_[propertyName] = value;
}

void UPowerDevice::requestProperties() {
    std::vector<std::string> propertyNames = {
        "BatteryLevel", "Capacity", "ChargeCycles", "Energy", "EnergyFull",
        "EnergyFullDesign", "EnergyRate", "HasHistory", "HasStatistics",
        "IconName", "IsPresent", "IsRechargeable", "Model", "NativePath",
        "Online", "Percentage", "PowerSupply", "Serial", "State", "Technology",
        "Temperature", "TimeToEmpty", "TimeToFull", "Type", "UpdateTime",
        "Vendor", "Voltage", "WarningLevel"
    };

    for (const auto& propertyName : propertyNames) {
        std::string value = getProperty(propertyName);
        updateProperty(propertyName, value);
    }
}

void UPowerDevice::printProperties() const {
    for (const auto& [name, value] : properties_) {
        std::cout << name << ": " << value << std::endl;
    }
}

class UPowerWakeups {
public:
    UPowerWakeups(DBusConnection* connection);

    void requestData();
    void printData() const;

private:
    std::string getData();
    DBusConnection* connection_;
    std::vector<std::string> data_;
};

UPowerWakeups::UPowerWakeups(DBusConnection* connection)
    : connection_(connection) {}

std::string UPowerWakeups::getData() {
    DBusMessage* msg;
    DBusMessage* reply;
    DBusError error;
    dbus_error_init(&error);

    msg = dbus_message_new_method_call("org.freedesktop.UPower",
                                       "/org/freedesktop/UPower/Wakeups",
                                       "org.freedesktop.UPower.Wakeups",
                                       "GetData");

    if (!msg) {
        std::cerr << "Failed to create message" << std::endl;
        std::exit(1);
    }

    reply = dbus_connection_send_with_reply_and_block(connection_, msg, -1, &error);

    std::string dataValue;
    if (dbus_error_is_set(&error)) {
        std::cerr << "Error in D-Bus method call: " << error.message << std::endl;
        dbus_error_free(&error);
    } else if (reply) {
        DBusMessageIter args;
        if (dbus_message_iter_init(reply, &args)) {
            DBusMessageIter arrayIter;
            dbus_message_iter_recurse(&args, &arrayIter);
            while (dbus_message_iter_get_arg_type(&arrayIter) == DBUS_TYPE_STRUCT) {
                DBusMessageIter structIter;
                dbus_message_iter_recurse(&arrayIter, &structIter);
                while (dbus_message_iter_get_arg_type(&structIter) != DBUS_TYPE_INVALID) {
                    if (dbus_message_iter_get_arg_type(&structIter) == DBUS_TYPE_STRING) {
                        const char* value;
                        dbus_message_iter_get_basic(&structIter, &value);
                        dataValue += std::string(value) + " ";
                    }
                    dbus_message_iter_next(&structIter);
                }
                data_.push_back(dataValue);
                dataValue.clear();
                dbus_message_iter_next(&arrayIter);
            }
        }
        dbus_message_unref(reply);
    }

    dbus_message_unref(msg);
    return dataValue;
}

void UPowerWakeups::requestData() {
    getData();
}

void UPowerWakeups::printData() const {
    for (const auto& entry : data_) {
        std::cout << "Wakeup Data: " << entry << std::endl;
    }
}

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

DBusHandlerResult messageHandler(DBusConnection* connection, 
                                 DBusMessage* message, 
                                 void* user_data) {
    UPowerDevice* device = static_cast<UPowerDevice*>(user_data);

    const char* interface = dbus_message_get_interface(message);
    const char* member = dbus_message_get_member(message);

    if (strcmp(interface, "org.freedesktop.UPower.Device") == 0) {
        if (strcmp(member, "PropertiesChanged") == 0) {
            std::cout << "Device Properties Changed" << std::endl;
            // Handle properties change here if needed
        }
    } else if (strcmp(interface, "org.freedesktop.UPower.Wakeups") == 0) {
        if (strcmp(member, "DataChanged") == 0) {
            std::cout << "Wakeups Data Changed" << std::endl;
            // Handle wakeups data change here if needed
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

void addMessageFilter(DBusConnection* connection, UPowerDevice& device) {
    dbus_bus_add_match(connection, 
        "type='signal',interface='org.freedesktop.UPower.Device'", 
        nullptr);
    dbus_connection_add_filter(connection, messageHandler, &device, nullptr);
}

void addWakeupsMessageFilter(DBusConnection* connection, UPowerWakeups& wakeups) {
    dbus_bus_add_match(connection, 
        "type='signal',interface='org.freedesktop.UPower.Wakeups'", 
        nullptr);
    dbus_connection_add_filter(connection, messageHandler, &wakeups, nullptr);
}

void runMainLoop(DBusConnection* connection, std::vector<UPowerDevice>& devices, UPowerWakeups& wakeups) {
    while (true) {
        dbus_connection_dispatch(connection);
        for (auto& device : devices) {
            device.requestProperties();
            device.printProperties();
        }
        wakeups.requestData();
        wakeups.printData();
        usleep(10000);
    }
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = connectToBus(&error);

    std::vector<UPowerDevice> devices = {
        //UPowerDevice(connection, "/org/freedesktop/UPower/devices/battery_BAT0"),
        //UPowerDevice(connection, "/org/freedesktop/UPower/devices/line_power_ADP1"),
        //UPowerDevice(connection, "/org/freedesktop/UPower/devices/DisplayDevice")
    };

    UPowerWakeups wakeups(connection);

    for (auto& device : devices) {
        device.requestProperties();
        device.printProperties();
        addMessageFilter(connection, device);
    }

    addWakeupsMessageFilter(connection, wakeups);

    runMainLoop(connection, devices, wakeups);

    dbus_connection_unref(connection);
    return 0;
}
