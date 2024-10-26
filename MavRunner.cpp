#include <iostream>
#include <libudev.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <functional>
#include <map>
#include <string>

#define UEVENT_BUFFER_SIZE 2048

struct USBDeviceInfo {
    std::string vendor_id;
    std::string product_id;
    std::string manufacturer;
    std::string product;

    USBDeviceInfo(const std::string& vendor, const std::string& product, const std::string& manuf, const std::string& prod) 
        : vendor_id(vendor), product_id(product), manufacturer(manuf), product(prod) {}

    void print_info() const {
        std::cout << "USB Device Info:" << std::endl;
        std::cout << "Vendor ID: " << vendor_id << std::endl;
        std::cout << "Product ID: " << product_id << std::endl;
        std::cout << "Manufacturer: " << manufacturer << std::endl;
        std::cout << "Product: " << product << std::endl;
        std::cout << "-----------------------" << std::endl;
    }
};


using EventCallback = std::function<void(const std::map<std::string, std::string>&)>;

std::map<std::string, std::string> parse_event_data(const char* buffer, ssize_t length) {
    std::map<std::string, std::string> event_data;

    for (const char* ptr = buffer; ptr < buffer + length; ptr += strlen(ptr) + 1) {
        std::string entry(ptr);
        size_t equal_pos = entry.find('=');

        if (equal_pos != std::string::npos) {
            std::string key = entry.substr(0, equal_pos);
            std::string value = entry.substr(equal_pos + 1);
            event_data[key] = value;
        }
    }

    return event_data;
}


void handle_device_event(int sock_fd, EventCallback callback) {
    char buffer[UEVENT_BUFFER_SIZE];
    ssize_t length = recv(sock_fd, buffer, sizeof(buffer), 0);
    if (length < 0) {
        std::cerr << "Failed to receive message" << std::endl;
        return;
    }
    std::map<std::string, std::string> event_data = parse_event_data(buffer, length);
    callback(event_data);
}


void monitor_device_events(EventCallback callback) {
    // Step 1: Create a netlink socket
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }


    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 1; // Listen to the kernel event group

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(sock);
        return;
    }


    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Failed to create epoll instance" << std::endl;
        close(sock);
        return;
    }

    struct epoll_event event;
    event.events = EPOLLIN;  
    event.data.fd = sock; 

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) < 0) {
        std::cerr << "Failed to add socket to epoll" << std::endl;
        close(sock);
        close(epoll_fd);
        return;
    }

    struct epoll_event events[10];
    while (true) {
        int num_events = epoll_wait(epoll_fd, events, 10, -1); 
        if (num_events < 0) {
            std::cerr << "Error in epoll_wait" << std::endl;
            break;
        }

        // Handle each event
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == sock) {
                handle_device_event(sock, callback);
            }
        }
    }

    close(sock);
    close(epoll_fd);
}

void list_existing_usb_devices() {
    struct udev* udev = udev_new();
    if (!udev) {
        std::cerr << "Cannot create udev context" << std::endl;
        return;
    }

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");  // We are interested only in USB devices
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* device = udev_device_new_from_syspath(udev, path);

        const char* vendor = udev_device_get_property_value(device, "ID_VENDOR_ID");
        const char* product = udev_device_get_property_value(device, "ID_MODEL_ID");
        const char* manufacturer = udev_device_get_property_value(device, "ID_VENDOR");
        const char* product_name = udev_device_get_property_value(device, "ID_MODEL");

        if (vendor && product && manufacturer && product_name) {
            USBDeviceInfo device_info(vendor, product, manufacturer, product_name);
            device_info.print_info();
        }

        udev_device_unref(device);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

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

int main(){
    
    return 0;
}