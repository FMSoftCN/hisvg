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

project(svg2png)

list(APPEND svg2png_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include
)

list(APPEND svg2png_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/svg2png.c
)

include_directories(${svg2png_INCLUDE_DIRECTORIES} ${GLIB_INCLUDE_DIRS} ${HIDOMLAYOUT_INCLUDE_DIRS} 
    ${HICairo_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIRS} ${PANGO_INCLUDE_DIRS} ${GIO_UNIX_INCLUDE_DIRS}
    ${MINIGUI_INCLUDE_DIRS})
link_directories(${HIDOMLAYOUT_LIBRARY_DIRS})
add_executable(svg2png ${svg2png_SOURCES})
target_link_libraries(svg2png hisvg ${GLIB_LIBRARIES}  
    ${HIDOMLAYOUT_LIBRARIES} ${HICairo_LIBRARIES} ${LIBXML2_LIBRARY} ${PANGO_LIBRARIES}
    ${GIO_UNIX_LIBRARIES} ${MINIGUI_LIBRARIES})

