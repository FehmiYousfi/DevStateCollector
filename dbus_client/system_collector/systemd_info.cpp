#include <dbus/dbus.h>
#include <iostream>
#include <string>
#include <vector>
#include <tinyxml2.h>

// Class to represent a node in the introspected data
class DBusNode {
public:
    std::string name;
    std::vector<std::string> interfaces;
    std::vector<DBusNode> children;

    DBusNode(const std::string& nodeName) : name(nodeName) {}

    void addInterface(const std::string& iface) {
        interfaces.push_back(iface);
    }

    void addChild(const DBusNode& childNode) {
        children.push_back(childNode);
    }

    void print(int indent = 0) const {
        std::string indentStr(indent, ' ');
        std::cout << indentStr << "Node: " << name << std::endl;
        for (const auto& iface : interfaces) {
            std::cout << indentStr << "  Interface: " << iface << std::endl;
        }
        for (const auto& child : children) {
            child.print(indent + 2);
        }
    }
};

DBusNode parseIntrospectionXML(const std::string& xmlData, const std::string& rootPath) {
    using namespace tinyxml2;

    DBusNode rootNode(rootPath);
    XMLDocument doc;
    if (doc.Parse(xmlData.c_str()) != XML_SUCCESS) {
        std::cerr << "Failed to parse XML data." << std::endl;
        return rootNode;
    }

    XMLElement* rootElement = doc.FirstChildElement("node");
    if (!rootElement) {
        std::cerr << "Invalid introspection data." << std::endl;
        return rootNode;
    }

    // Parse interfaces of the current node
    for (XMLElement* interfaceElement = rootElement->FirstChildElement("interface"); interfaceElement != nullptr; interfaceElement = interfaceElement->NextSiblingElement("interface")) {
        const char* ifaceName = interfaceElement->Attribute("name");
        if (ifaceName) {
            rootNode.addInterface(ifaceName);
        }
    }

    // Parse child nodes
    for (XMLElement* nodeElement = rootElement->FirstChildElement("node"); nodeElement != nullptr; nodeElement = nodeElement->NextSiblingElement("node")) {
        const char* nodeName = nodeElement->Attribute("name");
        if (nodeName) {
            std::string childPath = rootPath + "/" + nodeName;
            rootNode.addChild(DBusNode(childPath));
        }
    }

    return rootNode;
}

void introspect(DBusConnection* conn, const std::string& path, DBusNode& node) {
    DBusMessage* msg;
    msg = dbus_message_new_method_call("org.freedesktop.systemd1", // Target for the method call
                                       path.c_str(),               // Object path
                                       "org.freedesktop.DBus.Introspectable", // Interface to call
                                       "Introspect");              // Method name
    if (msg == nullptr) {
        std::cerr << "Failed to create D-Bus message.\n";
        return;
    }

    DBusMessage* reply;
    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, nullptr);
    dbus_message_unref(msg); // Unreference the message

    if (reply == nullptr) {
        std::cerr << "Failed to get a reply from D-Bus.\n";
        return;
    }

    const char* xml_data;
    if (!dbus_message_get_args(reply, nullptr, DBUS_TYPE_STRING, &xml_data, DBUS_TYPE_INVALID)) {
        std::cerr << "Failed to read arguments from the reply.\n";
    } else {
        DBusNode parsedNode = parseIntrospectionXML(xml_data, path);
        node = parsedNode;
    }

    dbus_message_unref(reply); // Unreference the reply message
}

int main() {
    DBusError error;
    dbus_error_init(&error);

    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error)) {
        std::cerr << "Error connecting to D-Bus: " << error.message << "\n";
        dbus_error_free(&error);
        return 1;
    }

    if (conn == nullptr) {
        std::cerr << "Failed to get a D-Bus connection.\n";
        return 1;
    }

    // Introspect the unit directory of systemd and parse the result
    DBusNode rootNode("/org/freedesktop/systemd1/unit");
    introspect(conn, "/org/freedesktop/systemd1/unit", rootNode);

    // Print the introspected data
    rootNode.print();

    // Close the D-Bus connection
    dbus_connection_unref(conn);

    return 0;
}
