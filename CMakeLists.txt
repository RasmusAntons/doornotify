cmake_minimum_required(VERSION 3.10)
project(doornotify C)

set(CMAKE_C_STANDARD 11)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/gnome-cmake/modules")

find_package(GLib)
find_package(GObject)
find_package(GTK3)

add_executable(doornotify doornotify.c config.h notification.h notification_linux.c notification_windows.c)
target_link_libraries(doornotify glib-2.0 gdk_pixbuf-2.0 gobject-2.0 notify gtk-3 paho-mqtt3c paho-mqtt3a)
