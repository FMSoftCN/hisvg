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

#ifndef HISVG_PAINT_SERVER_H
#define HISVG_PAINT_SERVER_H

#include <glib.h>
#include <cairo.h>
#include "hisvg-defs.h"

G_BEGIN_DECLS 

typedef struct _HiSVGGradientStop HiSVGGradientStop;
typedef struct _HiSVGGradientStops HiSVGGradientStops;
typedef struct _HiSVGLinearGradient HiSVGLinearGradient;
typedef struct _HiSVGRadialGradient HiSVGRadialGradient;
typedef struct _HiSVGPattern HiSVGPattern;
typedef struct _HiSVGSolidColor HiSVGSolidColor;

typedef struct _HiSVGPaintServer HiSVGPaintServer;

typedef struct _HiSVGPSCtx HiSVGPSCtx;

struct _HiSVGGradientStop {
    HiSVGNode super;
    double offset;
    guint32 rgba;
};

struct _HiSVGLinearGradient {
    HiSVGNode super;
    gboolean obj_bbox;
    cairo_matrix_t affine; /* user space to actual at time of gradient def */
    cairo_extend_t spread;
    HiSVGLength x1, y1, x2, y2;
    guint32 current_color;
    gboolean has_current_color;
    int hasx1:1;
    int hasy1:1;
    int hasx2:1;
    int hasy2:1;
    int hasbbox:1;
    int hasspread:1;
    int hastransform:1;
    char *fallback;
};

struct _HiSVGRadialGradient {
    HiSVGNode super;
    gboolean obj_bbox;
    cairo_matrix_t affine; /* user space to actual at time of gradient def */
    cairo_extend_t spread;
    HiSVGLength cx, cy, r, fx, fy;
    guint32 current_color;
    gboolean has_current_color;
    int hascx:1;
    int hascy:1;
    int hasfx:1;
    int hasfy:1;
    int hasr:1;
    int hasspread:1;
    int hasbbox:1;
    int hastransform:1;
    char *fallback;
};

struct _HiSVGPattern {
    HiSVGNode super;
    gboolean obj_cbbox;
    gboolean obj_bbox;
    cairo_matrix_t affine; /* user space to actual at time of gradient def */
    HiSVGLength x, y, width, height;
    HiSVGViewBox vbox;
    unsigned int preserve_aspect_ratio;
    int hasx:1;
    int hasy:1;
    int hasvbox:1;
    int haswidth:1;
    int hasheight:1;
    int hasaspect:1;
    int hascbox:1;
    int hasbbox:1;
    int hastransform:1;
    char *fallback;
};

struct _HiSVGSolidColor {
    gboolean currentcolor;
    guint32 argb;
};

typedef struct _HiSVGSolidColor HiSVGPaintServerColor;
typedef enum _HiSVGPaintServerType HiSVGPaintServerType;
typedef union _HiSVGPaintServerCore HiSVGPaintServerCore;

union _HiSVGPaintServerCore {
    HiSVGSolidColor *color;
    char *iri;
};

enum _HiSVGPaintServerType {
    HISVG_PAINT_SERVER_SOLID,
    HISVG_PAINT_SERVER_IRI
};

struct _HiSVGPaintServer {
    int refcnt;
    HiSVGPaintServerType type;
    HiSVGPaintServerCore core;
};

/* Create a new paint server based on a specification string. */
G_GNUC_INTERNAL
HiSVGPaintServer	    *hisvg_paint_server_parse    (gboolean * inherit, const char *str);
G_GNUC_INTERNAL
void                 hisvg_paint_server_ref      (HiSVGPaintServer * ps);
G_GNUC_INTERNAL
void                 hisvg_paint_server_unref    (HiSVGPaintServer * ps);

G_GNUC_INTERNAL
HiSVGNode *hisvg_new_linear_gradient  (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_radial_gradient  (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_stop	        (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_pattern      (const char* name);
G_GNUC_INTERNAL
void hisvg_pattern_fix_fallback          (HiSVGDrawingCtx * ctx,
                                         HiSVGPattern * pattern);
G_GNUC_INTERNAL
void hisvg_linear_gradient_fix_fallback	(HiSVGDrawingCtx * ctx,
                                         HiSVGLinearGradient * grad);
G_GNUC_INTERNAL
void hisvg_radial_gradient_fix_fallback	(HiSVGDrawingCtx * ctx,
                                         HiSVGRadialGradient * grad);

G_END_DECLS

#endif
