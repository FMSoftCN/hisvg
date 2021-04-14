
cmake_minimum_required(VERSION 3.10)

project(hisvg)


list(APPEND hisvg_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include
)

list(APPEND hisvg_SOURCES
    hisvg-handle.c
    hisvg-cairo.c
)

include_directories(${hisvg_INCLUDE_DIRECTORIES} ${GLIB_INCLUDE_DIRS} ${HIDOMLAYOUT_INCLUDE_DIRS} ${Cairo_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS})
link_directories(${HIDOMLAYOUT_LIBRARY_DIRS})
add_library(hisvg SHARED ${hisvg_SOURCES})
target_link_libraries(hisvg ${GLIB_LIBRARIES}  ${HIDOMLAYOUT_LIBRARIES} ${Cairo_LIBRARIES} ${LIBXML2_LIBRARY})

