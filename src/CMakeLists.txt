#
# 
#                           IMPORTANT NOTICE
# 
#  The following open source license statement does not apply to any
#  entity in the Exception List published by FMSoft.
# 
#  For more information, please visit:
# 
#  https://www.fmsoft.cn/exception-list
# 
#
#   This file is part of HiDOMLayout. hiDOMLayout is a library to
#   maintain a DOM tree, lay out and stylize the DOM nodes by
#   using CSS (Cascaded Style Sheets).
#
#   Copyright (C) 2021 Beijing FMSoft Technologies Co., Ltd.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General License for more details.
#
#   You should have received a copy of the GNU Lesser General License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   Or,
#
#   As this program is a library, any link to this program must follow
#   GNU Lesser General License version 3 (LGPLv3). If you cannot accept
#   LGPLv3, you need to be licensed from FMSoft.
#
#   If you have got a commercial license of this program, please use it
#   under the terms and conditions of the commercial license.
#
#   For more information about the commercial license, please refer to
#   <http://www.minigui.com/blog/minigui-licensing-policy/>.
#


cmake_minimum_required(VERSION 3.10)

project(hisvg VERSION 1.0.1)

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
set_target_properties(hisvg PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
target_link_libraries(hisvg ${GLIB_LIBRARIES}  ${HIDOMLAYOUT_LIBRARIES} ${HICairo_LIBRARIES} ${LIBXML2_LIBRARY} ${MINIGUI_LIBRARIES} )

configure_file(hisvg.pc.in hisvg.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hisvg.pc DESTINATION lib/pkgconfig)
install(FILES ${CMAKE_SOURCE_DIR}/include/hisvg.h DESTINATION include)
install(TARGETS hisvg DESTINATION lib)
