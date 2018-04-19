cmake_minimum_required(VERSION 3.10)
project(doornotify C)

set(CMAKE_C_STANDARD 11)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/gnome-cmake/modules")

find_package(GLib)
find_package(GDKPixbuf)
find_package(GObject)

add_subdirectory(paho.mqtt.c)
include_directories(${CMAKE_SOURCE_DIR}/paho.mqtt.c/src)

add_executable(doornotify doornotify.c config.h)
target_link_libraries(doornotify glib-2.0 gdk_pixbuf-2.0 gobject-2.0 notify paho-mqtt3c paho-mqtt3a)