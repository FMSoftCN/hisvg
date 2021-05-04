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

#ifndef HISVG_FILTER_H
#define HISVG_FILTER_H

#include "hisvg-common.h"
#include "hisvg-defs.h"

G_BEGIN_DECLS 

typedef struct  {
    int x0, y0, x1, y1;
} HiSVGIRect;

typedef HiSVGCoordUnits HiSVGFilterUnits;

struct _HiSVGFilter {
    HiSVGNode super;
    HiSVGLength x, y, width, height;
    HiSVGFilterUnits filterunits;
    HiSVGFilterUnits primitiveunits;
};

G_GNUC_INTERNAL
cairo_surface_t *hisvg_filter_render (HiSVGFilter *self,
                                     cairo_surface_t *source,
                                     HiSVGDrawingCtx *context, 
                                     HiSVGBbox *dimentions, 
                                     char *channelmap);

G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter	    (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_blend                (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_convolve_matrix      (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_gaussian_blur        (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_offset               (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_merge                (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_merge_node           (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_color_matrix        (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_component_transfer   (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_node_component_transfer_function      (const char* name, char channel);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_erode                (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_composite            (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_flood                (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_displacement_map     (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_turbulence           (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_image                (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_diffuse_lighting	    (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_node_light_source	                    (const char* name, char type);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_specular_lighting    (const char* name);
G_GNUC_INTERNAL
HiSVGNode    *hisvg_new_filter_primitive_tile                 (const char* name);

G_END_DECLS

#endif
