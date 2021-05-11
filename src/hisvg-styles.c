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

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "hisvg-common.h"
#include "hisvg-private.h"
#include "hisvg-filter.h"
#include "hisvg-css.h"
#include "hisvg-styles.h"
#include "hisvg-shapes.h"
#include "hisvg-mask.h"
#include "hisvg-marker.h"

#define HISVG_DEFAULT_FONT "Times New Roman"

enum {
  SHAPE_RENDERING_AUTO = CAIRO_ANTIALIAS_DEFAULT,
  SHAPE_RENDERING_OPTIMIZE_SPEED = CAIRO_ANTIALIAS_NONE,
  SHAPE_RENDERING_CRISP_EDGES = CAIRO_ANTIALIAS_NONE,
  SHAPE_RENDERING_GEOMETRIC_PRECISION = CAIRO_ANTIALIAS_DEFAULT
};

enum {
  TEXT_RENDERING_AUTO = CAIRO_ANTIALIAS_DEFAULT,
  TEXT_RENDERING_OPTIMIZE_SPEED = CAIRO_ANTIALIAS_NONE,
  TEXT_RENDERING_OPTIMIZE_LEGIBILITY = CAIRO_ANTIALIAS_DEFAULT,
  TEXT_RENDERING_GEOMETRIC_PRECISION = CAIRO_ANTIALIAS_DEFAULT
};

typedef struct _StyleValueData {
    gchar *value;
    gboolean important;
} StyleValueData;

/*
 * _hisvg_cairo_matrix_init_shear: Set up a shearing matrix.
 * @dst: Where to store the resulting affine transform.
 * @theta: Shear angle in degrees.
 *
 * Sets up a shearing matrix. In the standard libart coordinate system
 * and a small value for theta, || becomes \\. Horizontal lines remain
 * unchanged.
 **/
static void
_hisvg_cairo_matrix_init_shear (cairo_matrix_t *dst, double theta)
{
  cairo_matrix_init (dst, 1., 0., tan (theta * M_PI / 180.0), 1., 0., 0);
}

static StyleValueData *
style_value_data_new (const gchar *value, gboolean important)
{
    StyleValueData *ret;

    ret = g_new (StyleValueData, 1);
    ret->value = g_strdup (value);
    ret->important = important;

    return ret;
}

static void
style_value_data_free (StyleValueData *value)
{
    if (!value)
        return;
    g_free (value->value);
    g_free (value);
}

gdouble
hisvg_viewport_percentage (gdouble width, gdouble height)
{
    return sqrt (width * height);
}

gdouble
hisvg_dpi_percentage (HiSVGHandle * ctx)
{
    return sqrt (ctx->priv->dpi_x * ctx->priv->dpi_y);
}

void
hisvg_state_init (HiSVGState * state)
{
    memset (state, 0, sizeof (HiSVGState));

    state->parent = NULL;
    cairo_matrix_init_identity (&state->affine);
    cairo_matrix_init_identity (&state->personal_affine);
    state->mask = NULL;
    state->opacity = 0xff;
    state->baseline_shift = 0.;
    state->current_color = 0xff000000; /* See bgo#764808; we don't inherit CSS
                                        * from the public API, so start off with
                                        * opaque black instead of transparent.
                                        */
    state->fill = hisvg_paint_server_parse (NULL, "#000");
    state->fill_opacity = 0xff;
    state->stroke_opacity = 0xff;
    state->stroke_width = _hisvg_css_parse_length ("1");
    state->miter_limit = 4;
    state->cap = CAIRO_LINE_CAP_BUTT;
    state->join = CAIRO_LINE_JOIN_MITER;
    state->stop_opacity = 0xff;
    state->fill_rule = CAIRO_FILL_RULE_WINDING;
    state->clip_rule = CAIRO_FILL_RULE_WINDING;
    state->enable_background = HISVG_ENABLE_BACKGROUND_ACCUMULATE;
    state->comp_op = CAIRO_OPERATOR_OVER;
    state->overflow = FALSE;
    state->flood_color = 0;
    state->flood_opacity = 255;

    state->font_family = g_strdup (HISVG_DEFAULT_FONT);
    state->font_size = _hisvg_css_parse_length ("12.0");
    state->font_style = HISVG_TEXT_STYLE_NORMAL;
    state->font_variant = HISVG_TEXT_VARIANT_NORMAL;
    state->font_weight = HISVG_TEXT_WEIGHT_NORMAL;
    state->font_stretch = HISVG_TEXT_STRETCH_NORMAL;
    state->text_dir = HISVG_TEXT_DIRECTION_LTR;
    state->text_gravity = HISVG_TEXT_GRAVITY_SOUTH;
    state->unicode_bidi = UNICODE_BIDI_NORMAL;
    state->text_anchor = TEXT_ANCHOR_START;
    state->letter_spacing = _hisvg_css_parse_length ("0.0");
    state->writing_mode = GRF_WRITING_MODE_HORIZONTAL_TB;
    state->visible = TRUE;
    state->cond_true = TRUE;
    state->filter = NULL;
    state->clip_path = NULL;
    state->startMarker = NULL;
    state->middleMarker = NULL;
    state->endMarker = NULL;

    state->has_baseline_shift = FALSE;
    state->has_current_color = FALSE;
    state->has_flood_color = FALSE;
    state->has_flood_opacity = FALSE;
    state->has_fill_server = FALSE;
    state->has_fill_opacity = FALSE;
    state->has_fill_rule = FALSE;
    state->has_clip_rule = FALSE;
    state->has_stroke_server = FALSE;
    state->has_stroke_opacity = FALSE;
    state->has_stroke_width = FALSE;
    state->has_miter_limit = FALSE;
    state->has_cap = FALSE;
    state->has_join = FALSE;
    state->has_dash = FALSE;
    state->has_dashoffset = FALSE;
    state->has_visible = FALSE;
    state->has_cond = FALSE;
    state->has_stop_color = FALSE;
    state->has_stop_opacity = FALSE;
    state->has_font_size = FALSE;
    state->has_font_family = FALSE;
    state->has_lang = FALSE;
    state->has_font_style = FALSE;
    state->has_font_variant = FALSE;
    state->has_font_weight = FALSE;
    state->has_font_stretch = FALSE;
    state->has_font_decor = FALSE;
    state->has_text_dir = FALSE;
    state->has_text_gravity = FALSE;
    state->has_unicode_bidi = FALSE;
    state->has_text_anchor = FALSE;
    state->has_letter_spacing = FALSE;
    state->has_startMarker = FALSE;
    state->has_middleMarker = FALSE;
    state->has_endMarker = FALSE;
    state->has_overflow = FALSE;

    state->shape_rendering_type = SHAPE_RENDERING_AUTO;
    state->has_shape_rendering_type = FALSE;
    state->text_rendering_type = TEXT_RENDERING_AUTO;
    state->has_text_rendering_type = FALSE;

    state->styles = g_hash_table_new_full (g_str_hash, g_str_equal,
                                           g_free, (GDestroyNotify) style_value_data_free);
}

void
hisvg_state_reinit (HiSVGState * state)
{
    HiSVGState *parent = state->parent;
    hisvg_state_finalize (state);
    hisvg_state_init (state);
    state->parent = parent;
}

typedef int (*InheritanceFunction) (int dst, int src);

void
hisvg_state_clone (HiSVGState * dst, const HiSVGState * src)
{
    gint i;
    HiSVGState *parent = dst->parent;

    hisvg_state_finalize (dst);

    *dst = *src;
    dst->parent = parent;
    dst->filter = g_strdup (src->filter);
    dst->mask = g_strdup (src->mask);
    dst->clip_path = g_strdup (src->clip_path);
    dst->font_family = g_strdup (src->font_family);
    dst->lang = g_strdup (src->lang);
    dst->startMarker = g_strdup (src->startMarker);
    dst->middleMarker = g_strdup (src->middleMarker);
    dst->endMarker = g_strdup (src->endMarker);
    hisvg_paint_server_ref (dst->fill);
    hisvg_paint_server_ref (dst->stroke);

    dst->styles = g_hash_table_ref (src->styles);

    if (src->dash.n_dash > 0) {
        dst->dash.dash = g_new (gdouble, src->dash.n_dash);
        for (i = 0; i < src->dash.n_dash; i++)
            dst->dash.dash[i] = src->dash.dash[i];
    }
}

/*
  This function is where all inheritance takes place. It is given a 
  base and a modifier state, as well as a function to determine
  how the base is modified and a flag as to whether things that can
  not be inherited are copied streight over, or ignored.
*/

static void
hisvg_state_inherit_run (HiSVGState * dst, const HiSVGState * src,
                        const InheritanceFunction function, const gboolean inherituninheritables)
{
    gint i;

    if (function (dst->has_baseline_shift, src->has_baseline_shift))
        dst->baseline_shift = src->baseline_shift;
    if (function (dst->has_current_color, src->has_current_color))
        dst->current_color = src->current_color;
    if (function (dst->has_flood_color, src->has_flood_color))
        dst->flood_color = src->flood_color;
    if (function (dst->has_flood_opacity, src->has_flood_opacity))
        dst->flood_opacity = src->flood_opacity;
    if (function (dst->has_fill_server, src->has_fill_server)) {
        hisvg_paint_server_ref (src->fill);
        if (dst->fill)
            hisvg_paint_server_unref (dst->fill);
        dst->fill = src->fill;
    }
    if (function (dst->has_fill_opacity, src->has_fill_opacity))
        dst->fill_opacity = src->fill_opacity;
    if (function (dst->has_fill_rule, src->has_fill_rule))
        dst->fill_rule = src->fill_rule;
    if (function (dst->has_clip_rule, src->has_clip_rule))
        dst->clip_rule = src->clip_rule;
    if (function (dst->overflow, src->overflow))
        dst->overflow = src->overflow;
    if (function (dst->has_stroke_server, src->has_stroke_server)) {
        hisvg_paint_server_ref (src->stroke);
        if (dst->stroke)
            hisvg_paint_server_unref (dst->stroke);
        dst->stroke = src->stroke;
    }
    if (function (dst->has_stroke_opacity, src->has_stroke_opacity))
        dst->stroke_opacity = src->stroke_opacity;
    if (function (dst->has_stroke_width, src->has_stroke_width))
        dst->stroke_width = src->stroke_width;
    if (function (dst->has_miter_limit, src->has_miter_limit))
        dst->miter_limit = src->miter_limit;
    if (function (dst->has_cap, src->has_cap))
        dst->cap = src->cap;
    if (function (dst->has_join, src->has_join))
        dst->join = src->join;
    if (function (dst->has_stop_color, src->has_stop_color))
        dst->stop_color = src->stop_color;
    if (function (dst->has_stop_opacity, src->has_stop_opacity))
        dst->stop_opacity = src->stop_opacity;
    if (function (dst->has_cond, src->has_cond))
        dst->cond_true = src->cond_true;
    if (function (dst->has_font_size, src->has_font_size))
        dst->font_size = src->font_size;
    if (function (dst->has_font_style, src->has_font_style))
        dst->font_style = src->font_style;
    if (function (dst->has_font_variant, src->has_font_variant))
        dst->font_variant = src->font_variant;
    if (function (dst->has_font_weight, src->has_font_weight))
        dst->font_weight = src->font_weight;
    if (function (dst->has_font_stretch, src->has_font_stretch))
        dst->font_stretch = src->font_stretch;
    if (function (dst->has_font_decor, src->has_font_decor))
        dst->font_decor = src->font_decor;
    if (function (dst->has_text_dir, src->has_text_dir))
        dst->text_dir = src->text_dir;
    if (function (dst->has_text_gravity, src->has_text_gravity))
        dst->text_gravity = src->text_gravity;
    if (function (dst->has_unicode_bidi, src->has_unicode_bidi))
        dst->unicode_bidi = src->unicode_bidi;
    if (function (dst->has_text_anchor, src->has_text_anchor))
        dst->text_anchor = src->text_anchor;
    if (function (dst->has_letter_spacing, src->has_letter_spacing))
        dst->letter_spacing = src->letter_spacing;
    if (function (dst->has_startMarker, src->has_startMarker)) {
        g_free (dst->startMarker);
        dst->startMarker = g_strdup (src->startMarker);
    }
    if (function (dst->has_middleMarker, src->has_middleMarker)) {
        g_free (dst->middleMarker);
        dst->middleMarker = g_strdup (src->middleMarker);
    }
    if (function (dst->has_endMarker, src->has_endMarker)) {
        g_free (dst->endMarker);
        dst->endMarker = g_strdup (src->endMarker);
    }
    if (function (dst->has_shape_rendering_type, src->has_shape_rendering_type))
            dst->shape_rendering_type = src->shape_rendering_type;
    if (function (dst->has_text_rendering_type, src->has_text_rendering_type))
            dst->text_rendering_type = src->text_rendering_type;

    if (function (dst->has_font_family, src->has_font_family)) {
        g_free (dst->font_family);      /* font_family is always set to something */
        dst->font_family = g_strdup (src->font_family);
    }

    if (function (dst->has_space_preserve, src->has_space_preserve))
        dst->space_preserve = src->space_preserve;

    if (function (dst->has_visible, src->has_visible))
        dst->visible = src->visible;

    if (function (dst->has_lang, src->has_lang)) {
        if (dst->has_lang)
            g_free (dst->lang);
        dst->lang = g_strdup (src->lang);
    }

    if (src->dash.n_dash > 0 && (function (dst->has_dash, src->has_dash))) {
        if (dst->has_dash)
            g_free (dst->dash.dash);

        dst->dash.dash = g_new (gdouble, src->dash.n_dash);
        dst->dash.n_dash = src->dash.n_dash;
        for (i = 0; i < src->dash.n_dash; i++)
            dst->dash.dash[i] = src->dash.dash[i];
    }

    if (function (dst->has_dashoffset, src->has_dashoffset)) {
        dst->dash.offset = src->dash.offset;
    }

    if (inherituninheritables) {
        g_free (dst->clip_path);
        dst->clip_path = g_strdup (src->clip_path);
        g_free (dst->mask);
        dst->mask = g_strdup (src->mask);
        g_free (dst->filter);
        dst->filter = g_strdup (src->filter);
        dst->enable_background = src->enable_background;
        dst->opacity = src->opacity;
        dst->comp_op = src->comp_op;
    }
}

/*
  reinherit is given dst which is the top of the state stack
  and src which is the layer before in the state stack from
  which it should be inherited from 
*/

static int
reinheritfunction (int dst, int src)
{
    if (!dst)
        return 1;
    return 0;
}

void
hisvg_state_reinherit (HiSVGState * dst, const HiSVGState * src)
{
    hisvg_state_inherit_run (dst, src, reinheritfunction, 0);
}

/*
  dominate is given dst which is the top of the state stack
  and src which is the layer before in the state stack from
  which it should be inherited from, however if anything is
  directly specified in src (the second last layer) it will
  override anything on the top layer, this is for overrides
  in use tags 
*/

static int
dominatefunction (int dst, int src)
{
    if (!dst || src)
        return 1;
    return 0;
}

void
hisvg_state_dominate (HiSVGState * dst, const HiSVGState * src)
{
    hisvg_state_inherit_run (dst, src, dominatefunction, 0);
}

/* copy everything inheritable from the src to the dst */

static int
clonefunction (int dst, int src)
{
    return 1;
}

void
hisvg_state_override (HiSVGState * dst, const HiSVGState * src)
{
    hisvg_state_inherit_run (dst, src, clonefunction, 0);
}

/*
  put something new on the inheritance stack, dst is the top of the stack, 
  src is the state to be integrated, this is essentially the opposite of
  reinherit, because it is being given stuff to be integrated on the top, 
  rather than the context underneath.
*/

static int
inheritfunction (int dst, int src)
{
    return src;
}

void
hisvg_state_inherit (HiSVGState * dst, const HiSVGState * src)
{
    hisvg_state_inherit_run (dst, src, inheritfunction, 1);
}

void
hisvg_state_finalize (HiSVGState * state)
{
    g_free (state->filter);
    g_free (state->mask);
    g_free (state->clip_path);
    g_free (state->font_family);
    g_free (state->lang);
    g_free (state->startMarker);
    g_free (state->middleMarker);
    g_free (state->endMarker);
    hisvg_paint_server_unref (state->fill);
    hisvg_paint_server_unref (state->stroke);

    if (state->dash.n_dash != 0)
        g_free (state->dash.dash);

    if (state->styles) {
        g_hash_table_unref (state->styles);
        state->styles = NULL;
    }
}

/* Parse a CSS2 style argument, setting the SVG context attributes. */
void
hisvg_parse_style_pair (HiSVGHandle * ctx,
                       HiSVGState * state,
                       const gchar * name,
                       const gchar * value,
                       gboolean important)
{
    if (name == NULL || value == NULL)
        return;

    if (g_str_equal (name, "xml:space")) {
        state->has_space_preserve = TRUE;
        if (g_str_equal (value, "default"))
            state->space_preserve = FALSE;
        else if (!g_str_equal (value, "preserve") == 0)
            state->space_preserve = TRUE;
        else
            state->space_preserve = FALSE;
    } else if (g_str_equal (name, "xml:lang")) {
        char *save = g_strdup (value);
        g_free (state->lang);
        state->lang = save;
        state->has_lang = TRUE;
    }
}

static void
hisvg_lookup_style_pair_to_inner_css_prop (HiSVGHandle * ctx, HiSVGState * state,
                              const char *key, HiSVGPropertyBag * atts)
{
    const char *value;

    if ((value = hisvg_property_bag_lookup (atts, key)) != NULL)
    {
        _hisvg_node_attribute_to_node_inner_css_properties(ctx, state->node, key, value);
    }
}

static void
hisvg_lookup_style_attribute_to_inline_style (HiSVGHandle * ctx, HiSVGState * state, HiSVGPropertyBag * atts)
{
    const char *value;

    if ((value = hisvg_property_bag_lookup (atts, "style")) != NULL)
    {
        HISVG_NODE_SET_STYLE(state->node, value);
    }
}

static void
hisvg_lookup_parse_style_pair (HiSVGHandle * ctx, HiSVGState * state,
                              const char *key, HiSVGPropertyBag * atts)
{
    const char *value;

    if ((value = hisvg_property_bag_lookup (atts, key)) != NULL)
    {
        hisvg_parse_style_pair (ctx, state, key, value, FALSE);
    }
}

/* take a pair of the form (fill="#ff00ff") and parse it as a style */
void
hisvg_parse_style_pairs (HiSVGHandle * ctx, HiSVGState * state, HiSVGPropertyBag * atts)
{
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "baseline-shift", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "clip-path", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "clip-rule", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "color", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "direction", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "display", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "enable-background", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "comp-op", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "fill", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "fill-opacity", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "fill-rule", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "filter", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "flood-color", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "flood-opacity", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "font-family", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "font-size", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "font-stretch", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "font-style", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "font-variant", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "font-weight", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "marker-end", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "mask", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "marker-mid", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "marker-start", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "opacity", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "overflow", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "shape-rendering", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "text-rendering", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stop-color", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stop-opacity", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-dasharray", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-dashoffset", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-linecap", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-linejoin", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-miterlimit", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-opacity", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "stroke-width", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "text-anchor", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "text-decoration", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "unicode-bidi", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "letter-spacing", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "visibility", atts);
    hisvg_lookup_style_pair_to_inner_css_prop (ctx, state, "writing-mode", atts);
    hisvg_lookup_style_attribute_to_inline_style(ctx, state, atts);
    hisvg_lookup_parse_style_pair (ctx, state, "xml:lang", atts);
    hisvg_lookup_parse_style_pair (ctx, state, "xml:space", atts);

    {
        /* TODO: this conditional behavior isn't quite correct, and i'm not sure it should reside here */
        gboolean cond_true, has_cond;

        cond_true = hisvg_eval_switch_attributes (atts, &has_cond);

        if (has_cond) {
            state->cond_true = cond_true;
            state->has_cond = TRUE;
        }
    }
}

static gboolean
parse_style_value (const gchar *string, gchar **value, gboolean *important)
{
    gchar **strings;

    strings = g_strsplit (string, "!", 2);

    if (strings == NULL || strings[0] == NULL) {
        g_strfreev (strings);
        return FALSE;
    }

    if (strings[1] != NULL && strings[2] == NULL &&
        g_str_equal (g_strstrip (strings[1]), "important")) {
        *important = TRUE;
    } else {
        *important = FALSE;
    }

    *value = g_strdup (g_strstrip (strings[0]));

    g_strfreev (strings);

    return TRUE;
}

/* Split a CSS2 style into individual style arguments, setting attributes
   in the SVG context.
   
   It's known that this is _way_ out of spec. A more complete CSS2
   implementation will happen later.
*/
void
hisvg_parse_style (HiSVGHandle * ctx, HiSVGState * state, const char *str)
{
    gchar **styles;
    guint i;

    styles = g_strsplit (str, ";", -1);
    for (i = 0; i < g_strv_length (styles); i++) {
        gchar **values;
        values = g_strsplit (styles[i], ":", 2);
        if (!values)
            continue;

        if (g_strv_length (values) == 2) {
            gboolean important;
            gchar *style_value = NULL;
            gchar *first_value = values[0];
            gchar *second_value = values[1];
            gchar **split_list;

            /* Just remove single quotes in a trivial way.  No handling for any
             * special character inside the quotes is done.  This relates
             * especially to font-family names but cases with special characters
             * are rare.
             *
             * We need a real CSS parser, sigh.
             */
            split_list = g_strsplit (second_value, "'", -1);
            second_value = g_strjoinv(NULL, split_list);
            g_strfreev(split_list);

            if (parse_style_value (second_value, &style_value, &important))
                hisvg_parse_style_pair (ctx, state,
                                       g_strstrip (first_value),
                                       style_value,
                                       important);
            g_free (style_value);
            g_free (second_value);
        }
        g_strfreev (values);
    }
    g_strfreev (styles);
}

static void
hisvg_css_define_style (HiSVGHandle * ctx,
                       const gchar * selector,
                       const gchar * style_name,
                       const gchar * style_value,
                       gboolean important)
{
    GHashTable *styles;
    gboolean need_insert = FALSE;

    /* push name/style pair into HT */
    styles = g_hash_table_lookup (ctx->priv->css_props, selector);
    if (styles == NULL) {
        styles = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, (GDestroyNotify) style_value_data_free);
        g_hash_table_insert (ctx->priv->css_props, (gpointer) g_strdup (selector), styles);
        need_insert = TRUE;
    } else {
        StyleValueData *current_value;
        current_value = g_hash_table_lookup (styles, style_name);
        if (current_value == NULL || !current_value->important)
            need_insert = TRUE;
    }
    if (need_insert) {
        g_hash_table_insert (styles,
                             (gpointer) g_strdup (style_name),
                             (gpointer) style_value_data_new (style_value, important));
    }
}

void hisvg_parse_cssbuffer (HiSVGHandle* ctx, const char* buff, size_t buflen)
{
    if (ctx == NULL || buff == NULL || buflen == 0)
    {
        return;
    }

    size_t len = ctx->priv->css_buff_len + buflen;
    ctx->priv->css_buff = (uint8_t*)realloc(ctx->priv->css_buff, len);
    if (ctx->priv->css_buff == NULL)
    {
        fprintf(stderr, "no memory!\n");
        return;
    }
    memcpy(ctx->priv->css_buff + ctx->priv->css_buff_len, buff, buflen);
    ctx->priv->css_buff_len = len;
}

/* Parse an SVG transform string into an affine matrix. Reference: SVG
   working draft dated 1999-07-06, section 8.5. Return TRUE on
   success. */
gboolean
hisvg_parse_transform (cairo_matrix_t *dst, const char *src)
{
    int idx;
    char keyword[32];
    double args[6];
    int n_args;
    guint key_len;
    cairo_matrix_t affine;

    cairo_matrix_init_identity (dst);

    idx = 0;
    while (src[idx]) {
        /* skip initial whitespace */
        while (g_ascii_isspace (src[idx]))
            idx++;

        if (src[idx] == '\0')
            break;

        /* parse keyword */
        for (key_len = 0; key_len < sizeof (keyword); key_len++) {
            char c;

            c = src[idx];
            if (g_ascii_isalpha (c) || c == '-')
                keyword[key_len] = src[idx++];
            else
                break;
        }
        if (key_len >= sizeof (keyword))
            return FALSE;
        keyword[key_len] = '\0';

        /* skip whitespace */
        while (g_ascii_isspace (src[idx]))
            idx++;

        if (src[idx] != '(')
            return FALSE;
        idx++;

        for (n_args = 0;; n_args++) {
            char c;
            char *end_ptr;

            /* skip whitespace */
            while (g_ascii_isspace (src[idx]))
                idx++;
            c = src[idx];
            if (g_ascii_isdigit (c) || c == '+' || c == '-' || c == '.') {
                if (n_args == sizeof (args) / sizeof (args[0]))
                    return FALSE;       /* too many args */
                args[n_args] = g_ascii_strtod (src + idx, &end_ptr);
                idx = end_ptr - src;

                while (g_ascii_isspace (src[idx]))
                    idx++;

                /* skip optional comma */
                if (src[idx] == ',')
                    idx++;
            } else if (c == ')')
                break;
            else
                return FALSE;
        }
        idx++;

        /* ok, have parsed keyword and args, now modify the transform */
        if (!strcmp (keyword, "matrix")) {
            if (n_args != 6)
                return FALSE;

            cairo_matrix_init (&affine, args[0], args[1], args[2], args[3], args[4], args[5]);
            cairo_matrix_multiply (dst, &affine, dst);
        } else if (!strcmp (keyword, "translate")) {
            if (n_args == 1)
                args[1] = 0;
            else if (n_args != 2)
                return FALSE;
            cairo_matrix_init_translate (&affine, args[0], args[1]);
            cairo_matrix_multiply (dst, &affine, dst);
        } else if (!strcmp (keyword, "scale")) {
            if (n_args == 1)
                args[1] = args[0];
            else if (n_args != 2)
                return FALSE;
            cairo_matrix_init_scale (&affine, args[0], args[1]);
            cairo_matrix_multiply (dst, &affine, dst);
        } else if (!strcmp (keyword, "rotate")) {
            if (n_args == 1) {

                cairo_matrix_init_rotate (&affine, args[0] * M_PI / 180.);
                cairo_matrix_multiply (dst, &affine, dst);
            } else if (n_args == 3) {
                cairo_matrix_init_translate (&affine, args[1], args[2]);
                cairo_matrix_multiply (dst, &affine, dst);

                cairo_matrix_init_rotate (&affine, args[0] * M_PI / 180.);
                cairo_matrix_multiply (dst, &affine, dst);

                cairo_matrix_init_translate (&affine, -args[1], -args[2]);
                cairo_matrix_multiply (dst, &affine, dst);
            } else
                return FALSE;
        } else if (!strcmp (keyword, "skewX")) {
            if (n_args != 1)
                return FALSE;
            _hisvg_cairo_matrix_init_shear (&affine, args[0]);
            cairo_matrix_multiply (dst, &affine, dst);
        } else if (!strcmp (keyword, "skewY")) {
            if (n_args != 1)
                return FALSE;
            _hisvg_cairo_matrix_init_shear (&affine, args[0]);
            /* transpose the affine, given that we know [1] is zero */
            affine.yx = affine.xy;
            affine.xy = 0.;
            cairo_matrix_multiply (dst, &affine, dst);
        } else
            return FALSE;       /* unknown keyword */
    }
    return TRUE;
}

/**
 * hisvg_parse_transform_attr:
 * @ctx: HiSVG context.
 * @state: State in which to apply the transform.
 * @str: String containing transform.
 *
 * Parses the transform attribute in @str and applies it to @state.
 **/
static void
hisvg_parse_transform_attr (HiSVGHandle * ctx, HiSVGState * state, const char *str)
{
    cairo_matrix_t affine;

    if (hisvg_parse_transform (&affine, str)) {
        cairo_matrix_multiply (&state->personal_affine, &affine, &state->personal_affine);
        cairo_matrix_multiply (&state->affine, &affine, &state->affine);
    }
}


/**
 * hisvg_parse_style_attrs:
 * @ctx: HiSVG context.
 * @state: HiSVG state
 * @tag: (nullable): The SVG tag we're processing (eg: circle, ellipse), optionally %NULL
 * @klazz: (nullable): The space delimited class list, optionally %NULL
 * @atts: Attributes in SAX style.
 *
 * Parses style and transform attributes and modifies state at top of
 * stack.
 **/
void
hisvg_parse_style_attrs (HiSVGHandle * ctx,
                        HiSVGState * state,
                        const char *tag, const char *klazz, const char *id, HiSVGPropertyBag * atts)
{
    if (hisvg_property_bag_size (atts) == 0) {
        return;
    }

    const char* value = NULL;
    hisvg_parse_style_pairs (ctx, state, atts);
    if ((value = hisvg_property_bag_lookup (atts, "transform")) != NULL)
    {
        hisvg_parse_transform_attr (ctx, state, value);
    }
}

HiSVGState *
hisvg_current_state (HiSVGDrawingCtx * ctx)
{
    return ctx->state;
}

HiSVGState *
hisvg_state_parent (HiSVGState * state)
{
    return state->parent;
}

void
hisvg_state_free_all (HiSVGState * state)
{
    while (state) {
        HiSVGState *parent = state->parent;
        hisvg_state_finalize (state);
        g_slice_free (HiSVGState, state);
        state = parent;
    }
}

/**
 * hisvg_property_bag_new:
 * @atts: (array zero-terminated=1): list of alternating attributes
 *   and values
 * 
 * The property bag will NOT copy the attributes and values. If you need
 * to store them for later, use hisvg_property_bag_dup().
 * 
 * Returns: (transfer full): a new property bag
 */
HiSVGPropertyBag *
hisvg_property_bag_new (const char **atts)
{
    HiSVGPropertyBag *bag;
    int i;

    bag = g_hash_table_new (g_str_hash, g_str_equal);

    if (atts != NULL) {
        for (i = 0; atts[i] != NULL; i += 2)
            g_hash_table_insert (bag, (gpointer) atts[i], (gpointer) atts[i + 1]);
    }

    return bag;
}

/**
 * hisvg_property_bag_dup:
 * @bag: property bag to duplicate
 * 
 * Returns a copy of @bag that owns the attributes and values.
 * 
 * Returns: (transfer full): a new property bag
 */
HiSVGPropertyBag *
hisvg_property_bag_dup (HiSVGPropertyBag * bag)
{
    HiSVGPropertyBag *dup;
    GHashTableIter iter;
    gpointer key, value;

    dup = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    g_hash_table_iter_init (&iter, bag);
    while (g_hash_table_iter_next (&iter, &key, &value))
      g_hash_table_insert (dup, 
                           (gpointer) g_strdup ((char *) key),
                           (gpointer) g_strdup ((char *) value));

    return dup;
}

void
hisvg_property_bag_free (HiSVGPropertyBag * bag)
{
    g_hash_table_unref (bag);
}

const char *
hisvg_property_bag_lookup (HiSVGPropertyBag * bag, const char *key)
{
    return (const char *) g_hash_table_lookup (bag, (gconstpointer) key);
}

guint
hisvg_property_bag_size (HiSVGPropertyBag * bag)
{
    return g_hash_table_size (bag);
}

void
hisvg_property_bag_enumerate (HiSVGPropertyBag * bag, HiSVGPropertyBagEnumFunc func,
                             gpointer user_data)
{
    g_hash_table_foreach (bag, (GHFunc) func, user_data);
}

void
hisvg_state_push (HiSVGDrawingCtx * ctx)
{
    HiSVGState *data;
    HiSVGState *baseon;

    baseon = ctx->state;
    data = g_slice_new (HiSVGState);
    hisvg_state_init (data);

    if (baseon) {
        hisvg_state_reinherit (data, baseon);
        data->affine = baseon->affine;
        data->parent = baseon;
    }

    ctx->state = data;
}

void
hisvg_state_pop (HiSVGDrawingCtx * ctx)
{
    HiSVGState *dead_state = ctx->state;
    ctx->state = dead_state->parent;
    hisvg_state_finalize (dead_state);
    g_slice_free (HiSVGState, dead_state);
}

/*
  A function for modifying the top of the state stack depending on a 
  flag given. If that flag is 0, style and transform will inherit 
  normally. If that flag is 1, style will inherit normally with the
  exception that any value explicity set on the second last level
  will have a higher precedence than values set on the last level.
  If the flag equals two then the style will be overridden totally
  however the transform will be left as is. This is because of 
  patterns which are not based on the context of their use and are 
  rather based wholly on their own loading context. Other things
  may want to have this totally disabled, and a value of three will
  achieve this.
*/

void
hisvg_state_reinherit_top (HiSVGDrawingCtx * ctx, HiSVGState * state, int dominate)
{
    HiSVGState *current;

    if (dominate == 3)
        return;

    current = hisvg_current_state (ctx);
    /*This is a special domination mode for patterns, the transform
       is simply left as is, wheras the style is totally overridden */
    if (dominate == 2) {
        hisvg_state_override (current, state);
    } else {
        HiSVGState *parent= hisvg_state_parent (current);
        hisvg_state_clone (current, state);
        if (parent) {
            if (dominate)
                hisvg_state_dominate (current, parent);
            else
                hisvg_state_reinherit (current, parent);
            cairo_matrix_multiply (&current->affine,
                                   &current->affine,
                                   &parent->affine);
        }
    }
}

void
hisvg_state_reconstruct (HiSVGState * state, HiSVGNode * current)
{
    if (current == NULL)
        return;
    hisvg_state_reconstruct (state, HISVG_NODE_PARENT(current));
    hisvg_state_inherit (state, current->state);
}
