
cmake_minimum_required(VERSION 3.10)

project(hisvg)

add_definitions(-DHISVG_COMPILATION)


list(APPEND hisvg_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include
)

list(APPEND hisvg_SOURCES
    hisvg-base.c
    hisvg-base-file-util.c
    hisvg-cairo-clip.c
    hisvg-cairo-draw.c
    hisvg-cairo-render.c
    hisvg-css.c
    hisvg-defs.c
    hisvg-filter.c
    hisvg-gobject.c
    hisvg-image.c
    hisvg-marker.c
    hisvg-mask.c
    hisvg-paint-server.c
    hisvg-shapes.c
    hisvg-structure.c
    hisvg-styles.c
    hisvg-text.c

    hisvg-path.c
    hisvg-xml.c
    hisvg-io.c

    hisvg-cond.c

    hisvg-select.c
    hisvg-select-impl.c
    hisvg-text-helper.c
)

include_directories(${hisvg_INCLUDE_DIRECTORIES} ${GLIB_INCLUDE_DIRS} ${HIDOMLAYOUT_INCLUDE_DIRS} 
    ${HICairo_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS} ${PANGO_INCLUDE_DIRS} ${GIO_UNIX_INCLUDE_DIRS}
    ${MINIGUI_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
link_directories(${HIDOMLAYOUT_LIBRARY_DIRS})
add_library(hisvg SHARED ${hisvg_SOURCES})
target_link_libraries(hisvg ${GLIB_LIBRARIES}  ${HIDOMLAYOUT_LIBRARIES} ${HICairo_LIBRARIES} ${LIBXML2_LIBRARY} ${MINIGUI_LIBRARIES} )

configure_file(hisvg.pc.in hisvg.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hisvg.pc DESTINATION lib/pkgconfig)
install(FILES ${CMAKE_SOURCE_DIR}/include/hisvg.h DESTINATION include)
install(TARGETS hisvg DESTINATION lib)
