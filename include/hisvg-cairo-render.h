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

#ifndef HISVG_CAIRO_RENDER_H
#define HISVG_CAIRO_RENDER_H

#include "hisvg-private.h"
#include <cairo.h>

G_BEGIN_DECLS typedef struct _HiSVGCairoRender HiSVGCairoRender;

struct _HiSVGCairoRender {
    HiSVGRender super;
    cairo_t *cr;
    double width;
    double height;

    cairo_t *initial_cr;
    double offset_x;
    double offset_y;

    GList *cr_stack;

    HiSVGBbox bbox;
    GList *bb_stack;
    GList *surfaces_stack;
};

#define HISVG_CAIRO_RENDER(render) (_HISVG_RENDER_CIC ((render), HISVG_RENDER_TYPE_CAIRO, HiSVGCairoRender))

G_GNUC_INTERNAL
HiSVGCairoRender *hisvg_cairo_render_new		(cairo_t * cr, double width, double height);
G_GNUC_INTERNAL
void		hisvg_cairo_render_hisvg_handle	(cairo_t * cr, HiSVGHandle * handle);
G_GNUC_INTERNAL
HiSVGDrawingCtx *hisvg_cairo_new_drawing_ctx	(cairo_t * cr, HiSVGHandle * handle);
HiSVGDrawingCtx *hisvg_cairo_new_drawing_ctx_with_viewport	(cairo_t * cr, HiSVGHandle * handle, const HiSVGRect* viewport);

G_END_DECLS

#endif
