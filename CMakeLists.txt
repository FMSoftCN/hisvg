cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLIB 2.40.0 REQUIRED COMPONENTS gio gio-unix gobject gthread gmodule)
find_package(HiDomLayout REQUIRED)
find_package(hicairo REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Pango REQUIRED)
find_package(MiniGUI 5.0.2 REQUIRED)

set(MINIGUI_LIBRARIES ${MiniGUI_LIBRARIES})
set(MINIGUI_INCLUDE_DIRS ${MiniGUI_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(tests)

