#include <libudev.h>
#include <iostream>
#include <string>
#include <cstring>
#include <map>

std::string get_usb_class_description(const std::string& class_code) {
    static const std::map<std::string, std::string> usb_class_map = {
        {"00", "Device: Use class information in the Interface Descriptors"},
        {"01", "Interface: Audio"},
        {"02", "Both: Communications and CDC Control"},
        {"03", "Interface: HID (Human Interface Device)"},
        {"05", "Interface: Physical"},
        {"06", "Interface: Image"},
        {"07", "Interface: Printer"},
        {"08", "Interface: Mass Storage"},
        {"09", "Device: Hub"},
        {"0A", "Interface: CDC-Data"},
        {"0B", "Interface: Smart Card"},
        {"0D", "Interface: Content Security"},
        {"0E", "Interface: Video"},
        {"0F", "Interface: Personal Healthcare"},
        {"10", "Interface: Audio/Video Devices"},
        {"11", "Device: Billboard Device Class"},
        {"12", "Interface: USB Type-C Bridge Class"},
        {"13", "Interface: USB Bulk Display Protocol Device Class"},
        {"14", "Interface: MCTP over USB Protocol Endpoint Device Class"},
        {"3C", "Interface: I3C Device Class"},
        {"DC", "Both: Diagnostic Device"},
        {"E0", "Interface: Wireless Controller"},
        {"EF", "Both: Miscellaneous"},
        {"FE", "Interface: Application Specific"},
        {"FF", "Both: Vendor Specific"}
    };
    auto it = usb_class_map.find(class_code);
    if (it != usb_class_map.end()) {
        return it->second;
    } else {
        return "Unknown USB class";
    }
}
void find_dev_node_by_usb(const std::string& vendor_id, const std::string& product_id) {
    struct udev* udev = udev_new();
    if (!udev) {
        std::cerr << "Cannot create udev context" << std::endl;
        return;
    }

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb"); 
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* device = udev_device_new_from_syspath(udev, path);
        const char* dev_vendor = udev_device_get_sysattr_value(device, "idVendor");
        const char* dev_product = udev_device_get_sysattr_value(device, "idProduct");
        const char* dev_class = udev_device_get_sysattr_value(device, "bDeviceClass");

        if (dev_vendor && dev_product && vendor_id == dev_vendor && product_id == dev_product) {
            std::cout << "Found matching USB device!" << std::endl;
            const char* dev_node = udev_device_get_devnode(device);
            if (dev_node) {
                std::cout << "Device Node under /dev: " << dev_node << std::endl;
            } else {
                std::cout << "No /dev node associated with this device." << std::endl;
            }
            if (dev_class) {
                std::string class_description = get_usb_class_description(dev_class);
                std::cout << "USB Device Class (" << dev_class << "): " << class_description << std::endl;
                if (std::strcmp(dev_class, "02") == 0) {
                    struct udev_enumerate* tty_enum = udev_enumerate_new(udev);
                    udev_enumerate_add_match_subsystem(tty_enum, "tty");
                    udev_enumerate_scan_devices(tty_enum);

                    struct udev_list_entry* tty_devices = udev_enumerate_get_list_entry(tty_enum);
                    struct udev_list_entry* tty_entry;

                    udev_list_entry_foreach(tty_entry, tty_devices) {
                        const char* tty_path = udev_list_entry_get_name(tty_entry);
                        struct udev_device* tty_device = udev_device_new_from_syspath(udev, tty_path);

                        const char* tty_parent_path = udev_device_get_syspath(udev_device_get_parent(tty_device));

                        if (tty_parent_path && std::strstr(tty_parent_path, path)) {
                            const char* tty_node = udev_device_get_devnode(tty_device);
                            if (tty_node) {
                                std::cout << "Associated tty device: " << tty_node << std::endl;
                            }
                        }
                        udev_device_unref(tty_device);
                    }
                    udev_enumerate_unref(tty_enum);
                }
            } else {
                std::cout << "No USB class information available." << std::endl;
            }
            std::cout << "-----------------------" << std::endl;
        }
        udev_device_unref(device);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Vendor ID> <Product ID>" << std::endl;
        return 1;
    }

    std::string vendor_id = argv[1];
    std::string product_id = argv[2];

    std::cout << "Searching for USB device with Vendor ID: " << vendor_id
              << " and Product ID: " << product_id << std::endl;
    find_dev_node_by_usb(vendor_id, product_id);

    return 0;
}
