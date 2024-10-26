# D-Bus Service Project

This project demonstrates a simple D-Bus service using `dbus-glib` and `glibmm`.

## Project Structure

- `CMakeLists.txt`: CMake build configuration file.
- `dbus/dbus_service.xml`: D-Bus introspection XML for the service.
- `include/dbus_service.h`: Header file defining the D-Bus service class.
- `src/dbus_service.cpp`: Source file implementing the D-Bus service.
- `src/main.cpp`: Entry point for the D-Bus service.

## Building

To build the project, use CMake:

```sh
mkdir build
cd build
cmake ..
make
