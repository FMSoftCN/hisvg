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

#include <string.h>

#include "hisvg-private.h"
#include "hisvg-styles.h"
#include "hisvg-text.h"
#include "hisvg-css.h"

#include "hisvg-shapes.h"

/* what we use for text rendering depends on what cairo has to offer */
#include <pango/pangocairo.h>

typedef struct _HiSVGNodeText HiSVGNodeText;

struct _HiSVGNodeText {
    HiSVGNode super;
    HiSVGLength x, y, dx, dy;
};

typedef struct _HiSVGNodeTref HiSVGNodeTref;

struct _HiSVGNodeTref {
    HiSVGNode super;
    char *link;
};
char *
hisvg_make_valid_utf8 (const char *str, int len)
{
    GString *string;
    const char *remainder, *invalid;
    int remaining_bytes, valid_bytes;

    string = NULL;
    remainder = str;

    if (len < 0)
        remaining_bytes = strlen (str);
    else
        remaining_bytes = len;

    while (remaining_bytes != 0) {
        if (g_utf8_validate (remainder, remaining_bytes, &invalid))
            break;
        valid_bytes = invalid - remainder;

        if (string == NULL)
            string = g_string_sized_new (remaining_bytes);

        g_string_append_len (string, remainder, valid_bytes);
        g_string_append_c (string, '?');

        remaining_bytes -= valid_bytes + 1;
        remainder = invalid + 1;
    }

    if (string == NULL)
        return len < 0 ? g_strndup (str, len) : g_strdup (str);

    g_string_append (string, remainder);

    return g_string_free (string, FALSE);
}

static GString *
_hisvg_text_chomp (HiSVGState *state, GString * in, gboolean * lastwasspace)
{
    GString *out;
    guint i;
    out = g_string_new (in->str);

    if (!state->space_preserve) {
        for (i = 0; i < out->len;) {
            if (out->str[i] == '\n')
                g_string_erase (out, i, 1);
            else
                i++;
        }

        for (i = 0; i < out->len; i++)
            if (out->str[i] == '\t')
                out->str[i] = ' ';

        for (i = 0; i < out->len;) {
            if (out->str[i] == ' ' && *lastwasspace)
                g_string_erase (out, i, 1);
            else {
                if (out->str[i] == ' ')
                    *lastwasspace = TRUE;
                else
                    *lastwasspace = FALSE;
                i++;
            }
        }
    }

    return out;
}


static void
_hisvg_node_text_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeText *text = (HiSVGNodeText *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "x")))
            text->x = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y")))
            text->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "dx")))
            text->dx = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "dy")))
            text->dy = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "text", klazz, id, atts);
    }
}

static void
 hisvg_text_render_text (HiSVGDrawingCtx * ctx, const char *text, gdouble * x, gdouble * y);


static void
 _hisvg_node_text_type_tspan (HiSVGNodeText * self, HiSVGDrawingCtx * ctx,
                             gdouble * x, gdouble * y, gboolean * lastwasspace,
                             gboolean usetextonly);

static void
 _hisvg_node_text_type_tref (HiSVGNodeTref * self, HiSVGDrawingCtx * ctx,
                            gdouble * x, gdouble * y, gboolean * lastwasspace,
                            gboolean usetextonly);

/* This function is responsible of selecting render for a text element including its children and giving it the drawing context */
static void
_hisvg_node_text_type_children (HiSVGNode * self, HiSVGDrawingCtx * ctx,
                               gdouble * x, gdouble * y, gboolean * lastwasspace,
                               gboolean usetextonly)
{
    guint i;

    hisvg_push_discrete_layer (ctx);
    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(self->base);
    while(child)
    {
        HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        HiSVGNodeType type = HISVG_NODE_TYPE (node);

        if (type == HISVG_NODE_TYPE_CHARS) {
            HiSVGNodeChars *chars = (HiSVGNodeChars *) node;
            GString *str = _hisvg_text_chomp (hisvg_current_state (ctx), chars->contents, lastwasspace);
            hisvg_text_render_text (ctx, str->str, x, y);
            g_string_free (str, TRUE);
        } else {
            if (usetextonly) {
                _hisvg_node_text_type_children (node, ctx, x, y, lastwasspace,
                                               usetextonly);
            } else {
                if (type == HISVG_NODE_TYPE_TSPAN) {
                    HiSVGNodeText *tspan = (HiSVGNodeText *) node;
                    hisvg_state_push (ctx);
                    _hisvg_node_text_type_tspan (tspan, ctx, x, y, lastwasspace,
                                                usetextonly);
                    hisvg_state_pop (ctx);
                } else if (type == HISVG_NODE_TYPE_TREF) {
                    HiSVGNodeTref *tref = (HiSVGNodeTref *) node;
                    _hisvg_node_text_type_tref (tref, ctx, x, y, lastwasspace,
                                               usetextonly);
                }
            }
        }
    }
    hisvg_pop_discrete_layer (ctx);
}

static int
 _hisvg_node_text_length_tref (HiSVGNodeTref * self, HiSVGDrawingCtx * ctx,
                              gdouble * x, gboolean * lastwasspace,
                              gboolean usetextonly);

static int
 _hisvg_node_text_length_tspan (HiSVGNodeText * self, HiSVGDrawingCtx * ctx,
                               gdouble * x, gboolean * lastwasspace,
                               gboolean usetextonly);

static gdouble hisvg_text_length_text_as_string (HiSVGDrawingCtx * ctx, const char *text);

static int
_hisvg_node_text_length_children (HiSVGNode * self, HiSVGDrawingCtx * ctx,
                                 gdouble * length, gboolean * lastwasspace,
                                 gboolean usetextonly)
{
    guint i;
    int out = FALSE;
    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(self->base);
    while(child)
    {
        HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        HiSVGNodeType type = HISVG_NODE_TYPE (node);

        hisvg_state_push (ctx);
        hisvg_state_reinherit_top (ctx, node->state, 0);
        if (type == HISVG_NODE_TYPE_CHARS) {
            HiSVGNodeChars *chars = (HiSVGNodeChars *) node;
            GString *str = _hisvg_text_chomp (hisvg_current_state (ctx), chars->contents, lastwasspace);
            *length += hisvg_text_length_text_as_string (ctx, str->str);
            g_string_free (str, TRUE);
        } else {
            if (usetextonly) {
                out = _hisvg_node_text_length_children(node, ctx, length,
                                                      lastwasspace,
                                                      usetextonly);
            } else {
                if (type == HISVG_NODE_TYPE_TSPAN) {
                    HiSVGNodeText *tspan = (HiSVGNodeText *) node;
                    out = _hisvg_node_text_length_tspan (tspan, ctx, length,
                                                        lastwasspace,
                                                        usetextonly);
                } else if (type == HISVG_NODE_TYPE_TREF) {
                    HiSVGNodeTref *tref = (HiSVGNodeTref *) node;
                    out = _hisvg_node_text_length_tref (tref, ctx, length,
                                                       lastwasspace,
                                                       usetextonly);
                }
            }
        }
        hisvg_state_pop (ctx);
        if (out)
            break;
    }
    return out;
}


static void
_hisvg_node_text_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    double x, y, dx, dy, length = 0;
    gboolean lastwasspace = TRUE;
    HiSVGNodeText *text = (HiSVGNodeText *) self;
    hisvg_state_reinherit_top (ctx, self->state, dominate);

    x = _hisvg_css_normalize_length (&text->x, ctx, 'h');
    y = _hisvg_css_normalize_length (&text->y, ctx, 'v');
    dx = _hisvg_css_normalize_length (&text->dx, ctx, 'h');
    dy = _hisvg_css_normalize_length (&text->dy, ctx, 'v');

    if (hisvg_current_state (ctx)->text_anchor != TEXT_ANCHOR_START) {
        _hisvg_node_text_length_children (self, ctx, &length, &lastwasspace, FALSE);
        if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_MIDDLE)
            length /= 2;
    }
    if (PANGO_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity)) {
        y -= length;
        if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_MIDDLE)
            dy /= 2;
        if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_END)
            dy = 0;
    } else {
        x -= length;
        if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_MIDDLE)
            dx /= 2;
        if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_END)
            dx = 0;
    }
    x += dx;
    y += dy;

    lastwasspace = TRUE;
    _hisvg_node_text_type_children (self, ctx, &x, &y, &lastwasspace, FALSE);
}

HiSVGNode *
hisvg_new_text (const char* name)
{
    HiSVGNodeText *text;
    text = g_new (HiSVGNodeText, 1);
    _hisvg_node_init (&text->super, HISVG_NODE_TYPE_TEXT, name);
    text->super.draw = _hisvg_node_text_draw;
    text->super.set_atts = _hisvg_node_text_set_atts;
    text->x = text->y = text->dx = text->dy = _hisvg_css_parse_length ("0");
    return &text->super;
}

static void
_hisvg_node_text_type_tspan (HiSVGNodeText * self, HiSVGDrawingCtx * ctx,
                            gdouble * x, gdouble * y, gboolean * lastwasspace,
                            gboolean usetextonly)
{
    double dx, dy, length = 0;
    hisvg_state_reinherit_top (ctx, self->super.state, 0);

    dx = _hisvg_css_normalize_length (&self->dx, ctx, 'h');
    dy = _hisvg_css_normalize_length (&self->dy, ctx, 'v');

    if (hisvg_current_state (ctx)->text_anchor != TEXT_ANCHOR_START) {
        gboolean lws = *lastwasspace;
        _hisvg_node_text_length_children (&self->super, ctx, &length, &lws,
                                         usetextonly);
        if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_MIDDLE)
            length /= 2;
    }

    if (self->x.factor != 'n') {
        *x = _hisvg_css_normalize_length (&self->x, ctx, 'h');
        if (!PANGO_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity)) {
            *x -= length;
            if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_MIDDLE)
                dx /= 2;
            if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_END)
                dx = 0;
        }
    }
    *x += dx;

    if (self->y.factor != 'n') {
        *y = _hisvg_css_normalize_length (&self->y, ctx, 'v');
        if (PANGO_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity)) {
            *y -= length;
            if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_MIDDLE)
                dy /= 2;
            if (hisvg_current_state (ctx)->text_anchor == TEXT_ANCHOR_END)
                dy = 0;
        }
    }
    *y += dy;
    _hisvg_node_text_type_children (&self->super, ctx, x, y, lastwasspace,
                                   usetextonly);
}

static int
_hisvg_node_text_length_tspan (HiSVGNodeText * self,
                              HiSVGDrawingCtx * ctx, gdouble * length,
                              gboolean * lastwasspace, gboolean usetextonly)
{
    if (self->x.factor != 'n' || self->y.factor != 'n')
        return TRUE;

    if (PANGO_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity))
        *length += _hisvg_css_normalize_length (&self->dy, ctx, 'v');
    else
        *length += _hisvg_css_normalize_length (&self->dx, ctx, 'h');

    return _hisvg_node_text_length_children (&self->super, ctx, length,
                                             lastwasspace, usetextonly);
}

static void
_hisvg_node_tspan_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeText *text = (HiSVGNodeText *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "x")))
            text->x = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y")))
            text->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "dx")))
            text->dx = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "dy")))
            text->dy = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "tspan", klazz, id, atts);
    }
}

HiSVGNode *
hisvg_new_tspan (const char* name)
{
    HiSVGNodeText *text;
    text = g_new (HiSVGNodeText, 1);
    _hisvg_node_init (&text->super, HISVG_NODE_TYPE_TSPAN, name);
    text->super.set_atts = _hisvg_node_tspan_set_atts;
    text->x.factor = text->y.factor = 'n';
    text->dx = text->dy = _hisvg_css_parse_length ("0");
    return &text->super;
}

static void
_hisvg_node_text_type_tref (HiSVGNodeTref * self, HiSVGDrawingCtx * ctx,
                           gdouble * x, gdouble * y, gboolean * lastwasspace,
                           gboolean usetextonly)
{
    HiSVGNode *link;

    if (self->link == NULL)
      return;
    link = hisvg_acquire_node (ctx, self->link);
    if (link == NULL)
      return;

    _hisvg_node_text_type_children (link, ctx, x, y, lastwasspace,
                                                    TRUE);

    hisvg_release_node (ctx, link);
}

static int
_hisvg_node_text_length_tref (HiSVGNodeTref * self, HiSVGDrawingCtx * ctx, gdouble * x,
                             gboolean * lastwasspace, gboolean usetextonly)
{
    gboolean result;
    HiSVGNode *link;

    if (self->link == NULL)
      return FALSE;
    link = hisvg_acquire_node (ctx, self->link);
    if (link == NULL)
      return FALSE;

    result = _hisvg_node_text_length_children (link, ctx, x, lastwasspace, TRUE);

    hisvg_release_node (ctx, link);

    return result;
}

static void
hisvg_node_tref_free (HiSVGNode * node)
{
    HiSVGNodeTref *self = (HiSVGNodeTref *) node;
    g_free (self->link);
    _hisvg_node_free (node);
}

static void
_hisvg_node_tref_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *value;
    HiSVGNodeTref *text = (HiSVGNodeTref *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "xlink:href"))) {
            g_free (text->link);
            text->link = g_strdup (value);
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }
    }
}

HiSVGNode *
hisvg_new_tref (const char* name)
{
    HiSVGNodeTref *text;
    text = g_new (HiSVGNodeTref, 1);
    _hisvg_node_init (&text->super, HISVG_NODE_TYPE_TREF, name);
    text->super.free = hisvg_node_tref_free;
    text->super.set_atts = _hisvg_node_tref_set_atts;
    text->link = NULL;
    return &text->super;
}

typedef struct _HiSVGTextLayout HiSVGTextLayout;

struct _HiSVGTextLayout {
    PangoLayout *layout;
    HiSVGDrawingCtx *ctx;
    TextAnchor anchor;
    gdouble x, y;
};

static void
hisvg_text_layout_free (HiSVGTextLayout * layout)
{
    g_object_unref (layout->layout);
    g_free (layout);
}

static PangoLayout *
hisvg_text_create_layout (HiSVGDrawingCtx * ctx,
                         HiSVGState * state, const char *text, PangoContext * context)
{
    PangoFontDescription *font_desc;
    PangoLayout *layout;
    PangoAttrList *attr_list;
    PangoAttribute *attribute;

    if (state->lang)
        pango_context_set_language (context, pango_language_from_string (state->lang));

    if (state->unicode_bidi == UNICODE_BIDI_OVERRIDE || state->unicode_bidi == UNICODE_BIDI_EMBED)
        pango_context_set_base_dir (context, state->text_dir);

    if (PANGO_GRAVITY_IS_VERTICAL (state->text_gravity))
        pango_context_set_base_gravity (context, state->text_gravity);

    font_desc = pango_font_description_copy (pango_context_get_font_description (context));

    if (state->font_family)
        pango_font_description_set_family_static (font_desc, state->font_family);

    pango_font_description_set_style (font_desc, state->font_style);
    pango_font_description_set_variant (font_desc, state->font_variant);
    pango_font_description_set_weight (font_desc, state->font_weight);
    pango_font_description_set_stretch (font_desc, state->font_stretch);
    pango_font_description_set_size (font_desc,
                                     _hisvg_css_normalize_font_size (state, ctx) *
                                     PANGO_SCALE / ctx->dpi_y * 72);

    layout = pango_layout_new (context);
    pango_layout_set_font_description (layout, font_desc);
    pango_font_description_free (font_desc);

    attr_list = pango_attr_list_new ();
    attribute = pango_attr_letter_spacing_new (_hisvg_css_normalize_length (&state->letter_spacing,
                                                                           ctx, 'h') * PANGO_SCALE);
    attribute->start_index = 0;
    attribute->end_index = G_MAXINT;
    pango_attr_list_insert (attr_list, attribute); 

    if (state->has_font_decor && text) {
        if (state->font_decor & TEXT_UNDERLINE) {
            attribute = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
            attribute->start_index = 0;
            attribute->end_index = -1;
            pango_attr_list_insert (attr_list, attribute);
        }
	if (state->font_decor & TEXT_STRIKE) {
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

    pango_layout_set_alignment (layout, (state->text_dir == PANGO_DIRECTION_LTR) ?
                                PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT);

    return layout;
}


static HiSVGTextLayout *
hisvg_text_layout_new (HiSVGDrawingCtx * ctx, HiSVGState * state, const char *text)
{
    HiSVGTextLayout *layout;

    if (ctx->pango_context == NULL)
        ctx->pango_context = ctx->render->create_pango_context (ctx);

    layout = g_new0 (HiSVGTextLayout, 1);

    layout->layout = hisvg_text_create_layout (ctx, state, text, ctx->pango_context);
    layout->ctx = ctx;

    layout->anchor = state->text_anchor;

    return layout;
}

void
hisvg_text_render_text (HiSVGDrawingCtx * ctx, const char *text, gdouble * x, gdouble * y)
{
    PangoContext *context;
    PangoLayout *layout;
    PangoLayoutIter *iter;
    HiSVGState *state;
    gint w, h;
    double offset_x, offset_y, offset;

    state = hisvg_current_state (ctx);

    /* Do not render the text if the font size is zero. See bug #581491. */
    if (state->font_size.length == 0)
        return;

    context = ctx->render->create_pango_context (ctx);
    layout = hisvg_text_create_layout (ctx, state, text, context);
    pango_layout_get_size (layout, &w, &h);
    iter = pango_layout_get_iter (layout);
    offset = pango_layout_iter_get_baseline (iter) / (double) PANGO_SCALE;
    offset += _hisvg_css_accumulate_baseline_shift (state, ctx);
    if (PANGO_GRAVITY_IS_VERTICAL (state->text_gravity)) {
        offset_x = -offset;
        offset_y = 0;
    } else {
        offset_x = 0;
        offset_y = offset;
    }
    pango_layout_iter_free (iter);
    ctx->render->render_pango_layout (ctx, layout, *x - offset_x, *y - offset_y);
    if (PANGO_GRAVITY_IS_VERTICAL (state->text_gravity))
        *y += w / (double)PANGO_SCALE;
    else
        *x += w / (double)PANGO_SCALE;

    g_object_unref (layout);
    g_object_unref (context);
}

static gdouble
hisvg_text_layout_width (HiSVGTextLayout * layout)
{
    gint width;

    pango_layout_get_size (layout->layout, &width, NULL);

    return width / (double)PANGO_SCALE;
}

static gdouble
hisvg_text_length_text_as_string (HiSVGDrawingCtx * ctx, const char *text)
{
    HiSVGTextLayout *layout;
    gdouble x;

    layout = hisvg_text_layout_new (ctx, hisvg_current_state (ctx), text);
    layout->x = layout->y = 0;

    x = hisvg_text_layout_width (layout);

    hisvg_text_layout_free (layout);
    return x;
}
