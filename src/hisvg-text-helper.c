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

typedef struct _HiSVGTextContext {
    struct _PangoContext* pango_ctx;
} HiSVGTextContext; 

typedef struct _HiSVGTextContextLayout {
    struct _PangoLayout* pango_layout;
    HiSVGTextContext* context;
} HiSVGTextContextLayout;

typedef struct _PangoRectangle HiSVGTextRectangle;

HiSVGTextContext* hisvg_create_text_context ()
{
    HiSVGTextContext* ctx = (HiSVGTextContext*) calloc(1, sizeof(HiSVGTextContext));
    ctx->pango_ctx = pango_font_map_create_context (pango_cairo_font_map_get_default ());
    return ctx;
}

void hisvg_text_context_set_resolution (HiSVGTextContext* context, double dpi)
{
    pango_cairo_context_set_resolution (context->pango_ctx, dpi);
}

HiSVGTextGravity hisvg_text_context_get_gravity (HiSVGTextContext* context)
{
    return pango_context_get_gravity (context->pango_ctx);
}

void hisvg_text_context_set_language (HiSVGTextContext* context, const char* language)
{
    pango_context_set_language (context->pango_ctx, pango_language_from_string(language));
}

void hisvg_text_context_set_base_dir (HiSVGTextContext* context, HiSVGTextDirection direction)
{
    pango_context_set_base_dir(context->pango_ctx, direction);
}

void hisvg_text_context_set_base_gravity (HiSVGTextContext* context, HiSVGTextGravity gravity)
{
    pango_context_set_base_gravity (context->pango_ctx, gravity);
}

HiSVGTextContextLayout* hisvg_text_context_layout_new (HiSVGTextContext* context)
{
    HiSVGTextContextLayout* layout = (HiSVGTextContextLayout*)calloc(1, sizeof(HiSVGTextContextLayout));
    layout->pango_layout = pango_layout_new (context->pango_ctx);
    layout->context = context;
    return  layout;
}

void hisvg_text_context_layout_get_size (HiSVGTextContextLayout* layout, int* width, int* height)
{
    pango_layout_get_size (layout->pango_layout, width, height);
}

int hisvg_text_context_layout_get_baseline (HiSVGTextContextLayout* layout)
{
    return pango_layout_get_baseline(layout->pango_layout);
}

void hisvg_text_context_layout_set_alignment (HiSVGTextContextLayout* layout, HiSVGTextAlignment alignment)
{
    return pango_layout_set_alignment (layout->pango_layout, alignment);
}

void hisvg_text_context_layout_set_attributes (HiSVGTextContextLayout* layout, HiSVGTextAttrList* attrs)
{
    return pango_layout_set_attributes (layout->pango_layout, attrs);
}

void hisvg_text_context_layout_set_text (HiSVGTextContextLayout* layout, const char* text, int length)
{
    return pango_layout_set_text (layout->pango_layout, text, length);
}

void hisvg_text_context_layout_set_font_description (HiSVGTextContextLayout* layout, const HiSVGFontDescription* desc)
{
    pango_layout_set_font_description (layout->pango_layout, desc);
}

HiSVGFontDescription* hisvg_text_context_get_font_description (HiSVGTextContext* context)
{
    return pango_context_get_font_description(context->pango_ctx);
}

HiSVGFontDescription* hisvg_font_description_copy (const HiSVGFontDescription* desc)
{
    return pango_font_description_copy(desc);
}

void hisvg_font_description_set_family_static (HiSVGFontDescription* desc,  const char* family)
{
    pango_font_description_set_family_static (desc, family);
}

void hisvg_font_description_set_style (HiSVGFontDescription* desc, HiSVGTextStyle style)
{
    pango_font_description_set_style (desc, style);
}

void hisvg_font_description_set_variant (HiSVGFontDescription* desc, HiSVGTextVariant variant)
{
    pango_font_description_set_variant (desc, variant);
}

void hisvg_font_description_set_weight (HiSVGFontDescription* desc, HiSVGTextWeight weight)
{
    pango_font_description_set_weight (desc, weight);
}

void hisvg_font_description_set_stretch (HiSVGFontDescription* desc, HiSVGTextStretch stretch)
{
    pango_font_description_set_stretch (desc, stretch);
}

void hisvg_font_description_set_size (HiSVGFontDescription* desc, gint size)
{
    pango_font_description_set_size (desc, size);
}

void hisvg_font_description_free (HiSVGFontDescription* desc)
{
    pango_font_description_free (desc);
}

HiSVGTextAttrList*  hisvg_text_attr_list_new (void)
{
    return pango_attr_list_new();
}

void hisvg_text_attr_list_unref (HiSVGTextAttrList* list)
{
    pango_attr_list_unref (list);
}

void hisvg_text_attr_list_insert (HiSVGTextAttrList* list, PangoAttribute* attr)
{
    pango_attr_list_insert (list, attr);
}

HiSVGTextAttribute* hisvg_text_attr_strikethrough_new (gboolean strikethrough)
{
    return pango_attr_strikethrough_new (strikethrough);
}

HiSVGTextAttribute* hisvg_text_attr_underline_new (PangoUnderline underline)
{
    return pango_attr_underline_new (underline);
}

HiSVGTextAttribute* hisvg_text_attr_letter_spacing_new (int letter_spacing)
{
    return pango_attr_letter_spacing_new (letter_spacing);
}

HiSVGTextContext* hisvg_text_layout_get_context (HiSVGTextContextLayout* layout)
{
//    return pango_layout_get_context(layout);
    return layout->context;
}

void hisvg_text_context_layout_get_extents (HiSVGTextContextLayout* layout, HiSVGTextRectangle* ink_rect, HiSVGTextRectangle* logical_rect)
{
    pango_layout_get_extents (layout->pango_layout, ink_rect, logical_rect);
}

double hisvg_text_gravity_to_rotation (HiSVGTextGravity gravity)
{
    return pango_gravity_to_rotation (gravity);
}

void hisvg_cairo_update_text_context (cairo_t* cr, HiSVGTextContext* context)
{
    pango_cairo_update_context (cr, context->pango_ctx);
}

void hisvg_cairo_show_layout (cairo_t* cr, HiSVGTextContextLayout* layout)
{
    pango_cairo_show_layout(cr, layout->pango_layout);
}

void hisvg_cairo_layout_path (cairo_t* cr, HiSVGTextContextLayout* layout)
{
    pango_cairo_layout_path(cr, layout->pango_layout);
}
