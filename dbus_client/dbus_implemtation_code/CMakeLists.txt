cmake_minimum_required(VERSION 3.10)
project(dbus_example)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(DBUSMM REQUIRED dbus-glib-1)

include_directories(${GLIBMM_INCLUDE_DIRS} ${DBUSMM_INCLUDE_DIRS})
link_directories(${GLIBMM_LIBRARY_DIRS} ${DBUSMM_LIBRARY_DIRS})

add_executable(dbus_server main.cpp)

target_link_libraries(dbus_server ${GLIBMM_LIBRARIES} ${DBUSMM_LIBRARIES})

