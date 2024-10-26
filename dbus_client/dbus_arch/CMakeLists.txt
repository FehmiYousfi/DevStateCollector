cmake_minimum_required(VERSION 3.10)
project(DBusService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(DBUS_GLIB REQUIRED dbus-glib-1)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GIO REQUIRED gio-2.0)

include_directories(
    ${DBUS_INCLUDE_DIRS}
    ${DBUS_GLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    include
)

link_directories(
    ${DBUS_LIBRARY_DIRS}
    ${DBUS_GLIB_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
    ${GLIBMM_LIBRARY_DIRS}
    ${GIO_LIBRARY_DIRS}
)

add_executable(server src/main.cpp src/dbus_service.cpp)

target_link_libraries(server
    ${DBUS_LIBRARIES}
    ${DBUS_GLIB_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${GIO_LIBRARIES}
)

configure_file(dbus/CustomTarget.conf.in ${CMAKE_BINARY_DIR}/CustomTarget.conf @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/CustomTarget.conf DESTINATION /etc/dbus-1/system.d)
install(FILES dbus/CustomTarget-introspection.xml DESTINATION /usr/share/dbus-1/interfaces)
