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

#include <string.h>

#include "hisvg-private.h"
#include "hisvg-styles.h"
#include "hisvg-text.h"
#include "hisvg-css.h"

#include "hisvg-shapes.h"

/* what we use for text rendering depends on what cairo has to offer */
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
    if (HISVG_TEXT_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity)) {
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
        if (!HISVG_TEXT_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity)) {
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
        if (HISVG_TEXT_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity)) {
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

    if (HISVG_TEXT_GRAVITY_IS_VERTICAL (hisvg_current_state (ctx)->text_gravity))
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
    HiSVGTextContextLayout *layout;
    HiSVGDrawingCtx *ctx;
    TextAnchor anchor;
    gdouble x, y;
};

static void
hisvg_text_layout_free (HiSVGTextLayout * layout)
{
    hisvg_text_context_layout_destroy (layout->layout);
    g_free (layout);
}

static HiSVGTextContextLayout *
hisvg_text_create_layout (HiSVGDrawingCtx * ctx,
                         HiSVGState * state, const char *text, HiSVGTextContext * context)
{
    HiSVGFontDescription *font_desc;
    HiSVGTextContextLayout *layout;

    double font_size = _hisvg_css_normalize_font_size (state, ctx) / ctx->dpi_y * 72;

    font_desc = hisvg_font_description_create(NULL, state->font_family,
            state->font_style, state->font_variant, state->font_weight,
            state->font_stretch, state->font_decor, font_size);

    int letter_spacing = _hisvg_css_normalize_length (&state->letter_spacing, 
            ctx, 'h') * HISVG_TEXT_SCALE;
    HiSVGTextAlignment alignment = (state->text_dir == HISVG_TEXT_DIRECTION_LTR) ?
        HISVG_TEXT_ALIGN_LEFT : HISVG_TEXT_ALIGN_RIGHT;
    layout = hisvg_text_context_layout_create (context, 
        letter_spacing, alignment, font_desc, state->font_decor, state->writing_mode, text);

    hisvg_font_description_destroy (font_desc);

    return layout;
}


static HiSVGTextLayout *
hisvg_text_layout_new (HiSVGDrawingCtx * ctx, HiSVGState * state, const char *text)
{
    HiSVGTextLayout *layout;

    if (ctx->text_context == NULL)
        ctx->text_context = ctx->render->create_text_context (ctx, state);

    layout = g_new0 (HiSVGTextLayout, 1);

    layout->layout = hisvg_text_create_layout (ctx, state, text, (HiSVGTextContext*)ctx->text_context);
    layout->ctx = ctx;

    layout->anchor = state->text_anchor;

    return layout;
}

void
hisvg_text_render_text (HiSVGDrawingCtx * ctx, const char *text, gdouble * x, gdouble * y)
{
    HiSVGTextContext *context;
    HiSVGTextContextLayout *layout;
    HiSVGState *state;
    gint w, h;
    double offset_x, offset_y, offset;
    int baseline;

    state = hisvg_current_state (ctx);

    /* Do not render the text if the font size is zero. See bug #581491. */
    if (state->font_size.length == 0)
        return;

    context = ctx->render->create_text_context (ctx, state);
    layout = hisvg_text_create_layout (ctx, state, text, context);
    hisvg_text_context_layout_get_size (layout, &w, &h);
    baseline = hisvg_text_context_layout_get_baseline(layout);
    offset = baseline / (double) HISVG_TEXT_SCALE;
    offset += _hisvg_css_accumulate_baseline_shift (state, ctx);
    if (HISVG_TEXT_GRAVITY_IS_VERTICAL (state->text_gravity)) {
        offset_x = -offset;
        offset_y = 0;
    } else {
        offset_x = 0;
        offset_y = offset;
    }
    ctx->render->render_text (ctx, layout, *x - offset_x, *y - offset_y);
    if (HISVG_TEXT_GRAVITY_IS_VERTICAL (state->text_gravity))
        *y += w / (double)HISVG_TEXT_SCALE;
    else
        *x += w / (double)HISVG_TEXT_SCALE;

    hisvg_text_context_layout_destroy (layout);
    hisvg_text_context_destroy (context);
}

static gdouble
hisvg_text_layout_width (HiSVGTextLayout * layout)
{
    gint width;

    hisvg_text_context_layout_get_size (layout->layout, &width, NULL);

    return width / (double)HISVG_TEXT_SCALE;
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
