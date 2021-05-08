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

#include <pango/pangocairo.h>
#include "hisvg-text-helper.h"

typedef struct _PangoFontMap HiSVGFontMap;
typedef struct _PangoContext HiSVGTextContext;
typedef struct _PangoLayout HiSVGTextContextLayout;
typedef struct _PangoRectangle HiSVGTextRectangle;

HiSVGFontMap* hisvg_font_map_get_default ()
{
    return pango_cairo_font_map_get_default ();
}

HiSVGTextContext* hisvg_create_text_context (HiSVGFontMap* fontmap)
{
    return pango_font_map_create_context (fontmap);
}

void hisvg_text_context_set_resolution (HiSVGTextContext* context, double dpi)
{
    pango_cairo_context_set_resolution (context, dpi);
}

HiSVGTextGravity hisvg_text_context_get_gravity (HiSVGTextContext* context)
{
    return pango_context_get_gravity (context);
}

HiSVGTextContext* hisvg_text_layout_get_context (HiSVGTextContextLayout* layout)
{
    return pango_layout_get_context(layout);
}

void hisvg_text_context_layout_get_extents (HiSVGTextContextLayout* layout, HiSVGTextRectangle* ink_rect, HiSVGTextRectangle* logical_rect)
{
    pango_layout_get_extents (layout, ink_rect, logical_rect);
}

double hisvg_text_gravity_to_rotation (HiSVGTextGravity gravity)
{
    return pango_gravity_to_rotation (gravity);
}

void hisvg_cairo_update_text_context (cairo_t* cr, HiSVGTextContext* context)
{
    pango_cairo_update_context (cr, context);
}

void hisvg_cairo_show_layout (cairo_t* cr, HiSVGTextContextLayout* layout)
{
    pango_cairo_show_layout(cr, layout);
}

void hisvg_cairo_layout_path (cairo_t* cr, HiSVGTextContextLayout* layout)
{
    pango_cairo_layout_path(cr, layout);
}
