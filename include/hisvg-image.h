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

#ifndef HISVG_IMAGE_H
#define HISVG_IMAGE_H

#include "hisvg-structure.h"

#include <cairo.h>

G_BEGIN_DECLS 

G_GNUC_INTERNAL
HiSVGNode *hisvg_new_image (const char* name);

typedef struct _HiSVGNodeImage HiSVGNodeImage;

struct _HiSVGNodeImage {
    HiSVGNode super;
    gint preserve_aspect_ratio;
    HiSVGLength x, y, w, h;
    cairo_surface_t *surface; /* a cairo image surface */
};

G_GNUC_INTERNAL
void hisvg_preserve_aspect_ratio (unsigned int aspect_ratio, double width,
                                 double height, double *w, double *h, double *x, double *y);
G_GNUC_INTERNAL
cairo_surface_t *hisvg_cairo_surface_new_from_href (HiSVGHandle *handle, const char *href, GError ** error);

G_END_DECLS

#endif                          /* HISVG_IMAGE_H */
