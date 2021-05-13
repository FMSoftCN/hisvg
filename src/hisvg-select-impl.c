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
#include "hisvg-select.h"

#define CSS_SELECT_PROP_FUNC_IMPL(pname)                                           \
  int32_t css_select_##pname (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value) { return 0; }

int32_t css_select_baseline_shift (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->baseline_shift)
    {
        case HL_BASELINE_SHIFT_BASELINE:
            state->has_baseline_shift = TRUE;
            state->baseline_shift = 0.;
            break;
        case HL_BASELINE_SHIFT_SUB:
            state->has_baseline_shift = TRUE;
            state->baseline_shift = -0.2;
            break;
        case HL_BASELINE_SHIFT_SUPER:
            state->has_baseline_shift = TRUE;
            state->baseline_shift = 0.4;
            break;
    }
    return 0;
}

int32_t css_select_clip_path (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->clip_path)
    {
        g_free (state->clip_path);
        state->clip_path = g_strdup(svg_value->clip_path);
    }
    return 0;
}

int32_t css_select_clip_rule (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    state->has_clip_rule = TRUE;
    switch (svg_value->clip_rule)
    {
        case HL_CLIP_RULE_NONZERO:
            state->clip_rule = CAIRO_FILL_RULE_WINDING;
            break;
        case HL_CLIP_RULE_EVENODD:
            state->clip_rule = CAIRO_FILL_RULE_EVEN_ODD;
            break;
    }
    return 0;
}

int32_t css_select_color (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    // FIXME: unset/inherited color==0
    if (svg_value->color  != 0)
    {
        state->has_current_color = 1;
        state->current_color = svg_value->color;
    }
    return 0;
}

int32_t css_select_direction (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    state->has_text_dir = TRUE;
    switch (svg_value->direction)
    {
        case HL_DIRECTION_LTR:
            state->text_dir = HISVG_TEXT_DIRECTION_LTR;
            break;
        case HL_DIRECTION_RTL:
            state->text_dir = HISVG_TEXT_DIRECTION_RTL;
            break;
        default:
            state->text_dir = HISVG_TEXT_DIRECTION_LTR;
            break;
    }
    return 0;
}

int32_t css_select_display (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->display == HL_DISPLAY_NONE)
    {
        state->has_visible = TRUE;
        state->visible = FALSE;
    }
}

int32_t css_select_enable_background (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->enable_background ==  HL_ENABLE_BACKGROUND_NEW)
    {
        state->enable_background = HISVG_ENABLE_BACKGROUND_NEW;
    }
    else
    {
        state->enable_background = HISVG_ENABLE_BACKGROUND_ACCUMULATE;
    }
}

int32_t css_select_comp_op (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->comp_op)
    {
        case HL_COMP_OP_CLEAR:
            state->comp_op = CAIRO_OPERATOR_CLEAR;
            break;
        case HL_COMP_OP_SRC:
            state->comp_op = CAIRO_OPERATOR_SOURCE;
            break;
        case HL_COMP_OP_DST:
            state->comp_op = CAIRO_OPERATOR_DEST;
            break;
        case HL_COMP_OP_SRC_OVER:
            state->comp_op = CAIRO_OPERATOR_OVER;
            break;
        case HL_COMP_OP_DST_OVER:
            state->comp_op = CAIRO_OPERATOR_DEST_OVER;
            break;
        case HL_COMP_OP_SRC_IN:
            state->comp_op = CAIRO_OPERATOR_IN;
            break;
        case HL_COMP_OP_DST_IN:
            state->comp_op = CAIRO_OPERATOR_DEST_IN;
            break;
        case HL_COMP_OP_SRC_OUT:
            state->comp_op = CAIRO_OPERATOR_OUT;
            break;
        case HL_COMP_OP_DST_OUT:
            state->comp_op = CAIRO_OPERATOR_DEST_OUT;
            break;
        case HL_COMP_OP_SRC_ATOP:
            state->comp_op = CAIRO_OPERATOR_ATOP;
            break;
        case HL_COMP_OP_DST_ATOP:
            state->comp_op = CAIRO_OPERATOR_DEST_ATOP;
            break;
        case HL_COMP_OP_XOR:
            state->comp_op = CAIRO_OPERATOR_XOR;
            break;
        case HL_COMP_OP_PLUS:
            state->comp_op = CAIRO_OPERATOR_ADD;
            break;
        case HL_COMP_OP_MULTIPLY:
            state->comp_op = CAIRO_OPERATOR_MULTIPLY;
            break;
        case HL_COMP_OP_SCREEN:
            state->comp_op = CAIRO_OPERATOR_SCREEN;
            break;
        case HL_COMP_OP_OVERLAY:
            state->comp_op = CAIRO_OPERATOR_OVERLAY;
            break;
        case HL_COMP_OP_DARKEN:
            state->comp_op = CAIRO_OPERATOR_DARKEN;
            break;
        case HL_COMP_OP_LIGHTEN:
            state->comp_op = CAIRO_OPERATOR_LIGHTEN;
            break;
        case HL_COMP_OP_COLOR_DODGE:
            state->comp_op = CAIRO_OPERATOR_COLOR_DODGE;
            break;
        case HL_COMP_OP_COLOR_BURN:
            state->comp_op = CAIRO_OPERATOR_COLOR_BURN;
            break;
        case HL_COMP_OP_HARD_LIGHT:
            state->comp_op = CAIRO_OPERATOR_HARD_LIGHT;
            break;
        case HL_COMP_OP_SOFT_LIGHT:
            state->comp_op = CAIRO_OPERATOR_SOFT_LIGHT;
            break;
        case HL_COMP_OP_DIFFERENCE:
            state->comp_op = CAIRO_OPERATOR_DIFFERENCE;
            break;
        case HL_COMP_OP_EXCLUSION:
            state->comp_op = CAIRO_OPERATOR_EXCLUSION;
            break;
        default:
            state->comp_op = CAIRO_OPERATOR_OVER;
            break;
    }
    return 0;
}

HiSVGPaintServer* hisvg_paint_server_iri (char *iri);
HiSVGPaintServer* hisvg_paint_server_solid (guint32 argb);
HiSVGPaintServer* hisvg_paint_server_solid_current_color (void);
int32_t css_select_fill (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    HiSVGPaintServer *fill = state->fill;
    switch (svg_value->fill_type)
    {
        case HL_FILL_INHERIT:
            state->has_fill_server = 0;
            state->fill = hisvg_paint_server_solid (0);
            hisvg_paint_server_unref (fill);
            break;
        case HL_FILL_NONE:
            state->has_fill_server = 1;
            state->fill = NULL;
            hisvg_paint_server_unref (fill);
            break;
        case HL_FILL_URI:
            state->has_fill_server = 1;
            state->fill = hisvg_paint_server_iri (strdup(svg_value->fill_string));
            hisvg_paint_server_unref (fill);
            break;
        case HL_FILL_CURRENT_COLOR:
            state->has_fill_server = 1;
            state->fill = hisvg_paint_server_solid_current_color ();
            hisvg_paint_server_unref (fill);
            break;
        case HL_FILL_SET_COLOR:
            state->has_fill_server = 1;
            state->fill = hisvg_paint_server_solid (svg_value->fill_color);
            hisvg_paint_server_unref (fill);
            break;
    }
    return 0;
}

int32_t css_select_fill_opacity (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->fill_opacity_type == HL_FILL_OPACITY_SET)
    {
        state->fill_opacity = (guint) floor (svg_value->fill_opacity * 255. + 0.5);
        state->has_fill_opacity = TRUE;
    }
    return 0;
}

int32_t css_select_fill_rule (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    state->has_fill_rule = TRUE;
    switch (svg_value->fill_rule)
    {
        case HL_FILL_RULE_NONZERO:
            state->fill_rule = CAIRO_FILL_RULE_WINDING;
            break;
        case HL_FILL_RULE_EVENODD:
            state->fill_rule = CAIRO_FILL_RULE_EVEN_ODD;
            break;
    }
    return 0;
}

int32_t css_select_filter (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->filter)
    {
        g_free (state->filter);
        state->filter = g_strdup(svg_value->filter);
    }
    return 0;
}

int32_t css_select_flood_color (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    // FIXME: unset/inherited flood_color==0
    if (svg_value->flood_color  != 0)
    {
        state->has_flood_color = 1;
        state->flood_color = svg_value->flood_color;
    }
    return 0;
}

int32_t css_select_flood_opacity (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->flood_opacity_type == HL_FLOOD_OPACITY_SET)
    {
        state->flood_opacity = (guint) floor (svg_value->flood_opacity * 255. + 0.5);
        state->has_flood_opacity = TRUE;
    }
    return 0;
}

int32_t css_select_font_family (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->font_family_type == HL_FONT_FAMILY_INHERIT)
    {
        return 0;
    }
    if (svg_value->font_family)
    {
        g_free (state->font_family);
        state->font_family = g_strdup(svg_value->font_family);
    }
    return 0;
}

int32_t css_select_font_size (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    state->has_font_size = TRUE;
    double pow_factor = 0.0;
    HiSVGLength out;
    switch (svg_value->font_size_type)
    {
        case HL_FONT_SIZE_INHERIT:
            state->has_font_size = FALSE;
            break;
        case HL_FONT_SIZE_LARGE:
            out.length = 0.0;
            out.factor = 'l';
            break;
        case HL_FONT_SIZE_SMALLER:
            out.length = 0.0;
            out.factor = 's';
            break;
        case HL_FONT_SIZE_XX_SMALL:
            pow_factor = -3.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_X_SMALL:
            pow_factor = -2.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_SMALL:
            pow_factor = -1.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_MEDIUM:
            pow_factor = 0.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_X_LARGE:
            pow_factor = 1.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_XX_LARGE:
            pow_factor = 2.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_LARGER:
            pow_factor = 3.0;
            out.length = 12.0 * pow (1.2, pow_factor) / 72.0;
            out.factor = 'i';
            break;
        case HL_FONT_SIZE_DIMENSION:
            out.length = svg_value->font_size;
            switch (svg_value->font_size_unit)
            {
                case HL_UNIT_PCT:
                    out.length = svg_value->font_size / 100;
                    out.factor = 'p';
                    break;

                case HL_UNIT_EM:
                    out.factor = 'e';
                    break;
                case HL_UNIT_EX:
                    out.factor = 'x';
                    break;
                case HL_UNIT_IN:
                    out.factor = 'i';
                    break;
                default:
                    out.factor = '\0';
                    break;
            }
    }

    if (state->has_font_size)
    {
        state->font_size = out;
    }

    return 0;
}

int32_t css_select_font_stretch (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->font_stretch)
    {
        case HL_FONT_STRETCH_INHERIT:
            state->font_stretch = HISVG_TEXT_STRETCH_NORMAL;
            break;
        case HL_FONT_STRETCH_NORMAL:
            state->font_stretch = HISVG_TEXT_STRETCH_NORMAL;
            break;
        case HL_FONT_STRETCH_EXPANDED:
        case HL_FONT_STRETCH_WIDER:
            state->font_stretch = HISVG_TEXT_STRETCH_EXPANDED;
            break;
        case HL_FONT_STRETCH_CONDENSED:
        case HL_FONT_STRETCH_NARROWER:
            state->font_stretch = HISVG_TEXT_STRETCH_CONDENSED;
            break;
        case HL_FONT_STRETCH_ULTRA_CONDENSED:
            state->font_stretch = HISVG_TEXT_STRETCH_ULTRA_EXPANDED;
            break;
        case HL_FONT_STRETCH_EXTRA_CONDENSED:
            state->font_stretch = HISVG_TEXT_STRETCH_EXTRA_EXPANDED;
            break;
        case HL_FONT_STRETCH_SEMI_CONDENSED:
            state->font_stretch = HISVG_TEXT_STRETCH_SEMI_CONDENSED;
            break;
        case HL_FONT_STRETCH_SEMI_EXPANDED:
            state->font_stretch = HISVG_TEXT_STRETCH_SEMI_EXPANDED;
            break;
        case HL_FONT_STRETCH_EXTRA_EXPANDED:
            state->font_stretch = HISVG_TEXT_STRETCH_EXTRA_CONDENSED;
            break;
        case HL_FONT_STRETCH_ULTRA_EXPANDED:
            state->font_stretch = HISVG_TEXT_STRETCH_ULTRA_CONDENSED;
            break;
    }
    return 0;
}

int32_t css_select_font_style (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->font_style)
    {
        case HL_FONT_STYLE_INHERIT:
            state->font_style = HISVG_TEXT_STYLE_NORMAL;
            break;
        case HL_FONT_STYLE_NORMAL:
            state->font_style = HISVG_TEXT_STYLE_NORMAL;
            break;
        case HL_FONT_STYLE_ITALIC:
            state->font_style = HISVG_TEXT_STYLE_ITALIC;
            break;
        case HL_FONT_STYLE_OBLIQUE:
            state->font_style = HISVG_TEXT_STYLE_OBLIQUE;
            break;
    }
    return 0;
}

int32_t css_select_font_variant (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch(svg_value->font_variant)
    {
        case HL_FONT_VARIANT_INHERIT:
            state->font_variant = HISVG_TEXT_VARIANT_NORMAL;
            break;
        case HL_FONT_VARIANT_NORMAL:
            state->font_variant = HISVG_TEXT_VARIANT_NORMAL;
            break;
        case HL_FONT_VARIANT_SMALL_CAPS:
            state->font_variant = HISVG_TEXT_VARIANT_SMALL_CAPS;
            break;
    }
    return 0;
}

int32_t css_select_font_weight (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch(svg_value->font_weight)
    {
        case HL_FONT_WEIGHT_INHERIT:
            state->font_weight = HISVG_TEXT_WEIGHT_NORMAL;
            break;
        case HL_FONT_WEIGHT_NORMAL:
            state->font_weight = HISVG_TEXT_WEIGHT_NORMAL;
            break;
        case HL_FONT_WEIGHT_BOLD:
            state->font_weight = HISVG_TEXT_WEIGHT_BOLD;
            break;
        case HL_FONT_WEIGHT_BOLDER:
            state->font_weight = HISVG_TEXT_WEIGHT_ULTRABOLD;
            break;
        case HL_FONT_WEIGHT_LIGHTER:
            state->font_weight = HISVG_TEXT_WEIGHT_LIGHT;
            break;
        case HL_FONT_WEIGHT_100:
            state->font_weight = (HiSVGTextWeight) 100;
            break;
        case HL_FONT_WEIGHT_200:
            state->font_weight = (HiSVGTextWeight) 200;
            break;
        case HL_FONT_WEIGHT_300:
            state->font_weight = (HiSVGTextWeight) 300;
            break;
        case HL_FONT_WEIGHT_400:
            state->font_weight = (HiSVGTextWeight) 400;
            break;
        case HL_FONT_WEIGHT_500:
            state->font_weight = (HiSVGTextWeight) 500;
            break;
        case HL_FONT_WEIGHT_600:
            state->font_weight = (HiSVGTextWeight) 600;
            break;
        case HL_FONT_WEIGHT_700:
            state->font_weight = (HiSVGTextWeight) 700;
            break;
        case HL_FONT_WEIGHT_800:
            state->font_weight = (HiSVGTextWeight) 800;
            break;
        case HL_FONT_WEIGHT_900:
            state->font_weight = (HiSVGTextWeight) 900;
            break;
    }
    return 0;
}

int32_t css_select_marker_end (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->marker_end)
    {
        g_free (state->endMarker);
        state->endMarker = g_strdup(svg_value->marker_end);
        state->has_endMarker = TRUE;
    }
    return 0;
}

int32_t css_select_mask (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->mask)
    {
        g_free (state->mask);
        state->mask = g_strdup(svg_value->mask);
    }
    return 0;
}

int32_t css_select_marker_mid (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->marker_mid)
    {
        g_free (state->middleMarker);
        state->middleMarker = g_strdup(svg_value->marker_mid);
        state->has_middleMarker = TRUE;
    }
    return 0;
}

int32_t css_select_marker_start (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->marker_start)
    {
        g_free (state->startMarker);
        state->startMarker = g_strdup(svg_value->marker_start);
        state->has_startMarker = TRUE;
    }
    return 0;
}

int32_t css_select_opacity (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->opacity_type == HL_OPACITY_SET)
    {
        state->opacity = (guint) floor (svg_value->opacity * 255. + 0.5);
    }
    return 0;
}

int32_t css_select_overflow (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch(svg_value->overflow)
    {
        case HL_OVERFLOW_VISIBLE:
        case HL_OVERFLOW_AUTO:
            state->overflow = TRUE;
            break;
        case HL_OVERFLOW_HIDDEN:
        case HL_OVERFLOW_SCROLL:
            state->overflow = FALSE;
            break;
        default:
            state->overflow = FALSE;
            break;
    }
    return 0;
}


enum {
  SHAPE_RENDERING_AUTO = CAIRO_ANTIALIAS_DEFAULT,
  SHAPE_RENDERING_OPTIMIZE_SPEED = CAIRO_ANTIALIAS_NONE,
  SHAPE_RENDERING_CRISP_EDGES = CAIRO_ANTIALIAS_NONE,
  SHAPE_RENDERING_GEOMETRIC_PRECISION = CAIRO_ANTIALIAS_DEFAULT
};

int32_t css_select_shape_rendering (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->shape_rendering)
    {
        case HL_SHAPE_RENDERING_INHERIT:
            state->has_shape_rendering_type = FALSE;
            state->shape_rendering_type = SHAPE_RENDERING_AUTO;
            break;
        case HL_SHAPE_RENDERING_AUTO:
        case HL_SHAPE_RENDERING_DEFAULT:
            state->has_shape_rendering_type = TRUE;
            state->shape_rendering_type = SHAPE_RENDERING_AUTO;
            break;
        case HL_SHAPE_RENDERING_OPTIMIZESPEED:
            state->has_shape_rendering_type = TRUE;
            state->shape_rendering_type = SHAPE_RENDERING_OPTIMIZE_SPEED;
            break;
        case HL_SHAPE_RENDERING_CRISPEDGES:
            state->has_shape_rendering_type = TRUE;
            state->shape_rendering_type = SHAPE_RENDERING_CRISP_EDGES;
            break;
        case HL_SHAPE_RENDERING_GEOMETRICPRECISION:
            state->has_shape_rendering_type = TRUE;
            state->shape_rendering_type = SHAPE_RENDERING_GEOMETRIC_PRECISION;
            break;
    }

    return 0;
}

enum {
  TEXT_RENDERING_AUTO = CAIRO_ANTIALIAS_DEFAULT,
  TEXT_RENDERING_OPTIMIZE_SPEED = CAIRO_ANTIALIAS_NONE,
  TEXT_RENDERING_OPTIMIZE_LEGIBILITY = CAIRO_ANTIALIAS_DEFAULT,
  TEXT_RENDERING_GEOMETRIC_PRECISION = CAIRO_ANTIALIAS_DEFAULT
};
int32_t css_select_text_rendering (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->text_rendering)
    {
        case HL_TEXT_RENDERING_INHERIT:
            state->has_text_rendering_type = FALSE;
            state->text_rendering_type = TEXT_RENDERING_AUTO;
            break;
        case HL_TEXT_RENDERING_AUTO:
            state->has_text_rendering_type = TRUE;
            state->text_rendering_type = TEXT_RENDERING_AUTO;
            break;
        case HL_TEXT_RENDERING_OPTIMIZESPEED:
            state->has_text_rendering_type = TRUE;
            state->text_rendering_type = TEXT_RENDERING_OPTIMIZE_SPEED;
            break;
        case HL_TEXT_RENDERING_GEOMETRICPRECISION: 
            state->has_text_rendering_type = TRUE;
            state->text_rendering_type = TEXT_RENDERING_GEOMETRIC_PRECISION;
            break;
        case HL_TEXT_RENDERING_OPTIMIZELEGIBILITY:
            state->has_text_rendering_type = TRUE;
            state->text_rendering_type = TEXT_RENDERING_OPTIMIZE_LEGIBILITY;
            break;
        case HL_TEXT_RENDERING_DEFAULT:
            state->has_text_rendering_type = TRUE;
            state->text_rendering_type = TEXT_RENDERING_AUTO;
            break;
    }
    return 0;
}

int32_t css_select_stop_color (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    // FIXME: unset/inherited stop_color==0
    if (svg_value->stop_color  != 0)
    {
        state->has_stop_color = 1;
        state->stop_color = svg_value->stop_color;
    }
    return 0;
}

int32_t css_select_stop_opacity (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->stop_opacity_type == HL_STOP_OPACITY_SET)
    {
        state->stop_opacity = (guint) floor (svg_value->stop_opacity * 255. + 0.5);
        state->has_stop_opacity = TRUE;
    }
    return 0;
}

int32_t css_select_stroke (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    HiSVGPaintServer *stroke = state->stroke;
    switch (svg_value->stroke_type)
    {
        case HL_STROKE_INHERIT:
            state->has_stroke_server = 0;
            state->stroke = hisvg_paint_server_solid (0);
            hisvg_paint_server_unref (stroke);
            break;
        case HL_STROKE_NONE:
            state->has_stroke_server = 1;
            state->stroke = NULL;
            hisvg_paint_server_unref (stroke);
            break;
        case HL_STROKE_URI:
            state->has_stroke_server = 1;
            state->stroke = hisvg_paint_server_iri (svg_value->stroke_string);
            hisvg_paint_server_unref (stroke);
            break;
        case HL_STROKE_CURRENT_COLOR:
            state->has_stroke_server = 1;
            state->stroke = hisvg_paint_server_solid_current_color ();
            hisvg_paint_server_unref (stroke);
            break;
        case HL_STROKE_SET_COLOR:
            state->has_stroke_server = 1;
            state->stroke = hisvg_paint_server_solid (svg_value->stroke_color);
            hisvg_paint_server_unref (stroke);
            break;
    }
    return 0;
}

int32_t css_select_stroke_dasharray (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    size_t count = svg_value->stroke_dasharray_count;
    if(count == 0)
    {
        return 0;
    }

    state->has_dash = TRUE;
    state->dash.n_dash = count;
    state->dash.dash = g_new (double, state->dash.n_dash);
    for (int i=0; i < count; i++)
    {
        state->dash.dash[i] = svg_value->stroke_dasharray[i];
    }

    return 0;
}

int32_t css_select_stroke_dashoffset (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->stroke_dashoffset_type != HL_STROKE_DASHOFFSET_SET)
    {
        return 0;
    }

    HiSVGLength out;
    if (svg_value->stroke_dashoffset_unit == HL_UNIT_PCT)
    {
        out.length = svg_value->stroke_dashoffset / 100;
        out.factor = 'p';
    }
    else
    {
        out.length = svg_value->stroke_dashoffset;
        out.factor = '\0';
    }

    state->dash.offset = out;
    state->has_dashoffset = TRUE;
    if (state->dash.offset.length < 0.)
        state->dash.offset.length = 0.;
    return 0;
}

int32_t css_select_stroke_linecap (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch(svg_value->stroke_linecap)
    {
        case HL_STROKE_LINECAP_INHERIT:
            state->has_cap = FALSE;
            state->cap = CAIRO_LINE_CAP_BUTT;
            break;
        case HL_STROKE_LINECAP_BUTT:
            state->has_cap = TRUE;
            state->cap = CAIRO_LINE_CAP_BUTT;
            break;
        case HL_STROKE_LINECAP_ROUND:
            state->has_cap = TRUE;
            state->cap = CAIRO_LINE_CAP_ROUND;
            break;
        case HL_STROKE_LINECAP_SQUARE:
            state->has_cap = TRUE;
            state->cap = CAIRO_LINE_CAP_SQUARE;
            break;
    }
    return 0;
}

int32_t css_select_stroke_linejoin (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch(svg_value->stroke_linejoin)
    {
        case HL_STROKE_LINEJOIN_INHERIT:
            state->has_join = FALSE;
            state->join = CAIRO_LINE_JOIN_MITER;
            break;
        case HL_STROKE_LINEJOIN_MITER:
            state->has_join = TRUE;
            state->join = CAIRO_LINE_JOIN_MITER;
            break;
        case HL_STROKE_LINEJOIN_ROUND:
            state->has_join = TRUE;
            state->join = CAIRO_LINE_JOIN_ROUND;
            break;
        case HL_STROKE_LINEJOIN_BEVEL:
            state->join = CAIRO_LINE_JOIN_BEVEL;
            state->has_join = TRUE;
            break;
    }
    return 0;
}

int32_t css_select_stroke_miterlimit (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->stroke_miterlimit_type == HL_STROKE_MITERLIMIT_SET)
    {
        state->has_miter_limit = TRUE;
        state->miter_limit = svg_value->stroke_miterlimit;
    }
    return 0;
}

int32_t css_select_stroke_opacity (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->stroke_opacity_type == HL_STROKE_OPACITY_SET)
    {
        state->stroke_opacity = (guint) floor (svg_value->stroke_opacity * 255. + 0.5);
        state->has_stroke_opacity = TRUE;
    }
    return 0;
}

int32_t css_select_stroke_width (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->stroke_width_type != HL_STROKE_WIDTH_SET)
    {
        return 0;
    }

    HiSVGLength out;
    if (svg_value->stroke_width_unit == HL_UNIT_PCT)
    {
        out.length = svg_value->stroke_width / 100;
        out.factor = 'p';
    }
    else
    {
        out.length = svg_value->stroke_width;
        out.factor = '\0';
    }

    state->stroke_width = out;
    state->has_stroke_width = TRUE;
    return 0;
}

int32_t css_select_text_anchor (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->text_anchor)
    {
        case HL_TEXT_ANCHOR_INHERIT:
            state->has_text_anchor = FALSE;
            state->text_anchor = TEXT_ANCHOR_START;
            break;
        case HL_TEXT_ANCHOR_START:
            state->has_text_anchor = TRUE;
            state->text_anchor = TEXT_ANCHOR_START;
            break;
        case HL_TEXT_ANCHOR_MIDDLE:
            state->has_text_anchor = TRUE;
            state->text_anchor = TEXT_ANCHOR_MIDDLE;
            break;
        case HL_TEXT_ANCHOR_END:
            state->has_text_anchor = TRUE;
            state->text_anchor = TEXT_ANCHOR_END;
            break;
    }
    return 0;
}

int32_t css_select_text_decoration (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->text_decoration)
    {
        case HL_TEXT_DECORATION_INHERIT:
            state->has_font_decor = FALSE;
            state->font_decor = TEXT_NORMAL;
            break;
        case HL_TEXT_DECORATION_NONE:
            state->has_font_decor = FALSE;
            state->font_decor = TEXT_NORMAL;
            break;
        case HL_TEXT_DECORATION_BLINK:
            state->has_font_decor = TRUE;
            state->font_decor = TEXT_NORMAL;
            break;
        case HL_TEXT_DECORATION_LINE_THROUGH:
            state->has_font_decor = TRUE;
            state->font_decor |= TEXT_STRIKE;
            break;
        case HL_TEXT_DECORATION_OVERLINE:
            state->has_font_decor = TRUE;
            state->font_decor |= TEXT_OVERLINE;
            break;
        case HL_TEXT_DECORATION_UNDERLINE:
            state->has_font_decor = TRUE;
            state->font_decor |= TEXT_UNDERLINE;
            break;
    }
    return 0;
}

int32_t css_select_unicode_bidi (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->unicode_bidi)
    {
        case HL_UNICODE_BIDI_INHERIT:
            state->has_unicode_bidi = FALSE;
            state->unicode_bidi = UNICODE_BIDI_NORMAL;
            break;
        case HL_UNICODE_BIDI_EMBED:
            state->has_unicode_bidi = TRUE;
            state->unicode_bidi = UNICODE_BIDI_EMBED;
            break;
        case HL_UNICODE_BIDI_BIDI_OVERRIDE:
            state->has_unicode_bidi = TRUE;
            state->unicode_bidi = UNICODE_BIDI_OVERRIDE;
            break;
        case HL_UNICODE_BIDI_NORMAL:
        case HL_UNICODE_BIDI_ISOLATE:
        case HL_UNICODE_BIDI_ISOLATE_OVERRIDE:
        case HL_UNICODE_BIDI_PLAINTEXT:
            state->has_unicode_bidi = TRUE;
            state->unicode_bidi = UNICODE_BIDI_NORMAL;
            break;
    }
    return 0;
}

int32_t css_select_letter_spacing (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    if (svg_value->letter_spacing_type != HL_LETTER_SPACING_SET)
    {
        return 0;
    }

    HiSVGLength out;
    if (svg_value->letter_spacing_unit == HL_UNIT_PCT)
    {
        out.length = svg_value->letter_spacing / 100;
        out.factor = 'p';
    }
    else
    {
        out.length = svg_value->letter_spacing;
        out.factor = '\0';
    }

    state->letter_spacing = out;
    state->has_letter_spacing = TRUE;
    return 0;
}

int32_t css_select_visibility (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    switch (svg_value->visibility)
    {
        case HL_VISIBILITY_INHERIT:
            state->has_visible = FALSE;
            break;

        case HL_VISIBILITY_VISIBLE:
            state->has_visible = TRUE;
            state->visible = TRUE;
            break;
        case HL_VISIBILITY_HIDDEN:
        case HL_VISIBILITY_COLLAPSE:
            state->has_visible = TRUE;
            state->visible = FALSE;
            break;
    }
    return 0;
}

int32_t css_select_writing_mode (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)
{
    /* TODO: these aren't quite right... */
    switch (svg_value->writing_mode)
    {
        case HL_WRITING_MODE_INHERIT:
            state->text_dir = HISVG_TEXT_DIRECTION_LTR;
            state->has_text_dir = FALSE;
            state->text_gravity = HISVG_TEXT_GRAVITY_SOUTH;
            state->has_text_gravity = FALSE;
            state->writing_mode = GRF_WRITING_MODE_HORIZONTAL_TB;
            break;
        case HL_WRITING_MODE_HORIZONTAL_TB:
            state->has_text_dir = TRUE;
            state->has_text_gravity = TRUE;
            state->text_dir = HISVG_TEXT_DIRECTION_LTR;
            state->text_gravity = HISVG_TEXT_GRAVITY_SOUTH;
            state->writing_mode = GRF_WRITING_MODE_HORIZONTAL_TB;
            break;
        case HL_WRITING_MODE_VERTICAL_RL:
            state->has_text_dir = TRUE;
            state->has_text_gravity = TRUE;
            state->text_dir = HISVG_TEXT_DIRECTION_LTR;
            state->text_gravity = HISVG_TEXT_GRAVITY_EAST;
            state->writing_mode = GRF_WRITING_MODE_VERTICAL_RL;
            break;
        case HL_WRITING_MODE_VERTICAL_LR:
            state->has_text_dir = TRUE;
            state->has_text_gravity = TRUE;
            state->text_dir = HISVG_TEXT_DIRECTION_LTR;
            state->text_gravity = HISVG_TEXT_GRAVITY_WEST;
            state->writing_mode = GRF_WRITING_MODE_VERTICAL_LR;
            break;
    }
    return 0;
}
