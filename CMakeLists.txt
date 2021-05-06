cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLIB 2.40.0 REQUIRED COMPONENTS gio gio-unix gobject gthread gmodule)
find_package(HiDomLayout REQUIRED)
find_package(hicairo REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Pango REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)

