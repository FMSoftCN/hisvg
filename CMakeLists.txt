cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(GLIB 2.0 REQUIRED)
find_package(HiDomLayout REQUIRED)
find_package(Cairo REQUIRED)

add_subdirectory(src)

