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

    Copyright (C) 2004, 2005 Caleb Moore <c.moore@student.unsw.edu.au>
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

#ifndef HISVG_MARKER_H
#define HISVG_MARKER_H

#include "hisvg-structure.h"

G_BEGIN_DECLS 

typedef struct _HiSVGMarker HiSVGMarker;

struct _HiSVGMarker {
    HiSVGNode super;
    gboolean bbox;
    HiSVGLength refX, refY, width, height;
    double orient;
    gint preserve_aspect_ratio;
    gboolean orientAuto;
    HiSVGViewBox vbox;
};

G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_marker	    (const char* name);
G_GNUC_INTERNAL
void	     hisvg_render_markers    (HiSVGDrawingCtx *ctx, const cairo_path_t *path);

G_END_DECLS

#endif                          /* HISVG_MARKER_H */
