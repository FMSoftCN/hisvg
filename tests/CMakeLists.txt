cmake_minimum_required(VERSION 3.10)

project(svg2png)

list(APPEND svg2png_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include
)

list(APPEND svg2png_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/svg2png.c
)

include_directories(${svg2png_INCLUDE_DIRECTORIES} ${GLIB_INCLUDE_DIRS} ${HIDOMLAYOUT_INCLUDE_DIRS} 
    ${Cairo_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS} ${PANGO_INCLUDE_DIRS} ${GIO_UNIX_INCLUDE_DIRS}
    ${CROCO_INCLUDE_DIRS})
link_directories(${HIDOMLAYOUT_LIBRARY_DIRS})
add_executable(svg2png ${svg2png_SOURCES})
target_link_libraries(svg2png hisvg ${GLIB_LIBRARIES}  
    ${HIDOMLAYOUT_LIBRARIES} ${Cairo_LIBRARIES} ${LIBXML2_LIBRARY} ${PANGO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES} ${CROCO_LIBRARIES})

