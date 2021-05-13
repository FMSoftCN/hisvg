/////////////////////////////////////////////////////////////////////////////// //
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/**
 \verbatim

    This file is part of hiSVG. hiSVG is a  high performance SVG
    rendering library.

    Copyright (C) 2000 Eazel, Inc.
    Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
    Copyright (C) 2021 Beijing FMSoft Technologies Co., Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General License for more details.

    You should have received a copy of the GNU Lesser General License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Or,

    As this program is a library, any link to this program must follow
    GNU Lesser General License version 3 (LGPLv3). If you cannot accept
    LGPLv3, you need to be licensed from FMSoft.

    If you have got a commercial license of this program, please use it
    under the terms and conditions of the commercial license.

    For more information about the commercial license, please refer to
    <http://www.minigui.com/blog/minigui-licensing-policy/>.

 \endverbatim
 */

#ifndef HISVG_PATH_H
#define HISVG_PATH_H

#include <glib.h>
#include <cairo.h>

G_BEGIN_DECLS 

typedef struct {
    GArray *path_data;
    int     last_move_to_index;
} HiSVGPathBuilder;

G_GNUC_INTERNAL
void hisvg_path_builder_init (HiSVGPathBuilder *builder,
                             int n_elements);
G_GNUC_INTERNAL
void hisvg_path_builder_move_to (HiSVGPathBuilder *builder,
                                double x,
                                double y);
G_GNUC_INTERNAL
void hisvg_path_builder_line_to (HiSVGPathBuilder *builder,
                                double x,
                                double y);
G_GNUC_INTERNAL
void hisvg_path_builder_curve_to (HiSVGPathBuilder *builder,
                                 double x1,
                                 double y1,
                                 double x2,
                                 double y2,
                                 double x3,
                                 double y3);

G_GNUC_INTERNAL
void hisvg_path_builder_arc (HiSVGPathBuilder *builder,
                            double x1, double y1,
                            double rx, double ry,
                            double x_axis_rotation,
                            gboolean large_arc_flag, gboolean sweep_flag,
                            double x2, double y2);

G_GNUC_INTERNAL
void hisvg_path_builder_close_path (HiSVGPathBuilder *builder);
G_GNUC_INTERNAL
cairo_path_t *hisvg_path_builder_finish (HiSVGPathBuilder *builder);
G_GNUC_INTERNAL
cairo_path_t *hisvg_parse_path (const char *path_str);
G_GNUC_INTERNAL
void hisvg_cairo_path_destroy (cairo_path_t *path);

G_END_DECLS

#endif /* HISVG_PATH_H */
