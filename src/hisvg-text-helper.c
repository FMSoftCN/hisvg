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

#define HISVG_DEFAULT_FONT_FAMILY "serif"

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

HiSVGTextGravity hisvg_text_context_get_gravity (HiSVGTextContext* context)
{
    return pango_context_get_gravity (context->pango_ctx);
}

HiSVGTextContextLayout* hisvg_text_context_layout_create (HiSVGTextContext* context,
        int letter_spacing, HiSVGTextAlignment alignment, const HiSVGFontDescription* desc,
        int font_decor, const char* text)
{
    PangoLayout* layout = pango_layout_new (context->pango_ctx);
    HiSVGTextContextLayout* result = (HiSVGTextContextLayout*)calloc(1, sizeof(HiSVGTextContextLayout));
    result->pango_layout = layout;
    result->context = context;
    PangoAttrList* attr_list;
    PangoAttribute* attribute;

    // set font description
    PangoFontDescription* pdesc = pango_font_description_new();
    pango_font_description_set_family_static(pdesc, desc->family);
    pango_font_description_set_style(pdesc, desc->style);
    pango_font_description_set_variant(pdesc, desc->variant);
    pango_font_description_set_weight(pdesc, desc->weight);
    pango_font_description_set_stretch(pdesc, desc->stretch);
    pango_font_description_set_size(pdesc, desc->size);
    pango_layout_set_font_description (layout, pdesc);
    pango_font_description_free(pdesc);

    attr_list = pango_attr_list_new ();
    attribute = pango_attr_letter_spacing_new (letter_spacing);
    attribute->start_index = 0;
    attribute->end_index = G_MAXINT;
    pango_attr_list_insert (attr_list, attribute);
    if (text)
    {
        if (font_decor & TEXT_UNDERLINE) {
            attribute = pango_attr_underline_new (HISVG_TEXT_UNDERLINE_SINGLE);
            attribute->start_index = 0;
            attribute->end_index = -1;
            pango_attr_list_insert (attr_list, attribute);
        }
        if (font_decor & TEXT_STRIKE) {
            attribute = pango_attr_strikethrough_new (TRUE);
            attribute->start_index = 0;
            attribute->end_index = -1;
            pango_attr_list_insert (attr_list, attribute);
        }
    }

    pango_layout_set_attributes (layout, attr_list);
    pango_attr_list_unref (attr_list);

    if (text)
        pango_layout_set_text (layout, text, -1);
    else
        pango_layout_set_text (layout, NULL, 0);

    pango_layout_set_alignment (layout, alignment);

    return result;
}

void hisvg_text_context_layout_get_size (HiSVGTextContextLayout* layout, int* width, int* height)
{
    pango_layout_get_size (layout->pango_layout, width, height);
}

int hisvg_text_context_layout_get_baseline (HiSVGTextContextLayout* layout)
{
    return pango_layout_get_baseline(layout->pango_layout);
}

HiSVGFontDescription* hisvg_font_description_create (const char* type,
        const char* family, HiSVGTextStyle style, HiSVGTextVariant variant,
        HiSVGTextWeight weight, HiSVGTextStretch stretch, gint size,
        guint size_is_absolute
        )
{
    HiSVGFontDescription* desc = (HiSVGFontDescription*) calloc(1, sizeof(HiSVGFontDescription));
    desc->type = type ? strdup(type) : NULL;
    desc->family = family ? strdup(family) : strdup(HISVG_DEFAULT_FONT_FAMILY);
    desc->style = style;
    desc->variant = variant;
    desc->weight = weight;
    desc->stretch = stretch;
    desc->size = size;
    desc->size_is_absolute = size_is_absolute;
    return desc;
}

void hisvg_font_description_free (HiSVGFontDescription* desc)
{
    free(desc->type);
    free(desc->family);
    free(desc);
}

HiSVGTextContext* hisvg_text_layout_get_context (HiSVGTextContextLayout* layout)
{
    return layout->context;
}

void hisvg_text_context_layout_get_extents (HiSVGTextContextLayout* layout, HiSVGTextRectangle* ink_rect, HiSVGTextRectangle* logical_rect)
{
    PangoRectangle irect, lrect;
    pango_layout_get_extents (layout->pango_layout, &irect, &lrect);
    if (ink_rect)
    {
        ink_rect->x = irect.x;
        ink_rect->y = irect.y;
        ink_rect->width = irect.width;
        ink_rect->height = irect.height;
    }

    if (logical_rect)
    {
        logical_rect->x = lrect.x;
        logical_rect->y = lrect.y;
        logical_rect->width = lrect.width;
        logical_rect->height = lrect.height;
    }
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
