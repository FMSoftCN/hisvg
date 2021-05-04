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

    Copyright (C) 2021 Beijing FMSoft Technologies Co., Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Or,

    As this program is a library, any link to this program must follow
    GNU General Public License version 3 (GPLv3). If you cannot accept
    GPLv3, you need to be licensed from FMSoft.

    If you have got a commercial license of this program, please use it
    under the terms and conditions of the commercial license.

    For more information about the commercial license, please refer to
    <http://www.minigui.com/blog/minigui-licensing-policy/>.

 \endverbatim
 */
#ifndef HISVG_CAIRO_DRAW_H
#define HISVG_CAIRO_DRAW_H

#include "hisvg-private.h"

G_BEGIN_DECLS 

G_GNUC_INTERNAL
PangoContext    *hisvg_cairo_create_pango_context    (HiSVGDrawingCtx *ctx);
G_GNUC_INTERNAL
void         hisvg_cairo_render_pango_layout	    (HiSVGDrawingCtx *ctx, PangoLayout *layout, 
                                                 double x, double y);
G_GNUC_INTERNAL
void         hisvg_cairo_render_path             (HiSVGDrawingCtx *ctx, 
                                                 const cairo_path_t *path);
G_GNUC_INTERNAL
void         hisvg_cairo_render_surface          (HiSVGDrawingCtx *ctx, cairo_surface_t *surface,
                                                 double x, double y, double w, double h);
G_GNUC_INTERNAL
void         hisvg_cairo_push_discrete_layer	    (HiSVGDrawingCtx *ctx);
G_GNUC_INTERNAL
void         hisvg_cairo_pop_discrete_layer      (HiSVGDrawingCtx *ctx);
G_GNUC_INTERNAL
void         hisvg_cairo_add_clipping_rect       (HiSVGDrawingCtx *ctx,
                                                 double x, double y, double width, double height);
G_GNUC_INTERNAL
cairo_surface_t*hisvg_cairo_get_surface_of_node  (HiSVGDrawingCtx *ctx, HiSVGNode *drawable, 
                                                 double width, double height);

G_END_DECLS

#endif /*HISVG_CAIRO_DRAW_H */
