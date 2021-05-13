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
    Copyright (C) 2002 - 2005 Dom Lachowicz <cinamod@hotmail.com>
    Copyright (C) 2003 - 2005 Caleb Moore <c.moore@student.unsw.edu.au>
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

#include "hisvg-structure.h"
#include "hisvg-image.h"
#include "hisvg-css.h"
#include "string.h"

#include <stdio.h>

void
hisvg_node_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGState *state;
    GSList *stacksave;

    state = self->state;

    stacksave = ctx->drawsub_stack;
    if (stacksave) {
        if (stacksave->data != self)
            return;
        ctx->drawsub_stack = stacksave->next;
    }
    if (!state->visible)
        return;

    self->draw (self, ctx, dominate);
    ctx->drawsub_stack = stacksave;
}

/* generic function for drawing all of the children of a particular node */
void
_hisvg_node_draw_children (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    guint i;
    if (dominate != -1) {
        hisvg_state_reinherit_top (ctx, self->state, dominate);

        hisvg_push_discrete_layer (ctx);
    }
    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(self->base);
    while(child)
    {
        HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        hisvg_state_push (ctx);
        hisvg_node_draw (node, ctx, 0);
        hisvg_state_pop (ctx);
    }
    if (dominate != -1)
        hisvg_pop_discrete_layer (ctx);
}

/* generic function that doesn't draw anything at all */
static void
_hisvg_node_draw_nothing (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
}

static void
_hisvg_node_dont_set_atts (HiSVGNode * node, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
}

void
_hisvg_node_init (HiSVGNode * self,
                 HiSVGNodeType type,
                 const char* name)
{
    self->base = hilayout_element_node_create(name);
    HISVG_NODE_BIND_DOM_NODE(self, self->base);
    self->type = type;
    self->state = g_new (HiSVGState, 1);
    hisvg_state_init (self->state);
    self->state->node = self;
    self->inner_class_name = NULL;
    self->inner_class_value = NULL;
    self->inner_class = NULL;
    self->free = _hisvg_node_free;
    self->draw = _hisvg_node_draw_nothing;
    self->set_atts = _hisvg_node_dont_set_atts;
}

void
_hisvg_node_finalize (HiSVGNode * self)
{
    if (self->state != NULL) {
        hisvg_state_finalize (self->state);
        g_free (self->state);
    }
    hilayout_element_node_destroy(self->base);
    free(self->inner_class_name);
    free(self->inner_class_value);
    free(self->inner_class);
}

void
_hisvg_node_free (HiSVGNode * self)
{
    _hisvg_node_finalize (self);
    g_free (self);
}

static void
hisvg_node_group_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "g", klazz, id, atts);
    }
}

HiSVGNode *
hisvg_new_group (const char* name)
{
    HiSVGNodeGroup *group;
    group = g_new (HiSVGNodeGroup, 1);
    _hisvg_node_init (&group->super, HISVG_NODE_TYPE_GROUP, name);
    group->super.draw = _hisvg_node_draw_children;
    group->super.set_atts = hisvg_node_group_set_atts;
    return &group->super;
}

void
hisvg_pop_def_group (HiSVGHandle * ctx)
{
    g_assert (ctx->priv->currentnode != NULL);
    ctx->priv->currentnode = HISVG_NODE_PARENT(ctx->priv->currentnode);
}

void
hisvg_node_group_pack (HiSVGNode * self, HiSVGNode * child)
{
    HISVG_NODE_ADD_CHILD(self, child);
}

static gboolean
hisvg_node_is_ancestor (HiSVGNode * potential_ancestor, HiSVGNode * potential_descendant)
{
    /* work our way up the family tree */
    while (TRUE) {
        if (potential_ancestor == potential_descendant)
            return TRUE;
        else if (!HISVG_NODE_HAS_PARENT(potential_descendant))
            return FALSE;
        else
            potential_descendant = HISVG_NODE_PARENT(potential_descendant);
    }

    g_assert_not_reached ();
    return FALSE;
}

static void
hisvg_node_use_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGNodeUse *use = (HiSVGNodeUse *) self;
    HiSVGNode *child;
    HiSVGState *state;
    cairo_matrix_t affine;
    double x, y, w, h;
    x = _hisvg_css_normalize_length (&use->x, ctx, 'h');
    y = _hisvg_css_normalize_length (&use->y, ctx, 'v');
    w = _hisvg_css_normalize_length (&use->w, ctx, 'h');
    h = _hisvg_css_normalize_length (&use->h, ctx, 'v');

    hisvg_state_reinherit_top (ctx, self->state, dominate);

    if (use->link == NULL)
      return;
    child = hisvg_acquire_node (ctx, use->link);
    if (!child)
        return;
    else if (hisvg_node_is_ancestor (child, self)) {     /* or, if we're <use>'ing ourself */
        hisvg_release_node (ctx, child);
        return;
    }

    state = hisvg_current_state (ctx);
    if (HISVG_NODE_TYPE (child) != HISVG_NODE_TYPE_SYMBOL) {
        cairo_matrix_init_translate (&affine, x, y);
        cairo_matrix_multiply (&state->affine, &affine, &state->affine);

        hisvg_push_discrete_layer (ctx);
        hisvg_state_push (ctx);
        hisvg_node_draw (child, ctx, 1);
        hisvg_state_pop (ctx);
        hisvg_pop_discrete_layer (ctx);
    } else {
        HiSVGNodeSymbol *symbol = (HiSVGNodeSymbol *) child;

        if (symbol->vbox.active) {
            hisvg_preserve_aspect_ratio
                (symbol->preserve_aspect_ratio,
                 symbol->vbox.rect.width, symbol->vbox.rect.height,
                 &w, &h, &x, &y);

            cairo_matrix_init_translate (&affine, x, y);
            cairo_matrix_multiply (&state->affine, &affine, &state->affine);

            cairo_matrix_init_scale (&affine, w / symbol->vbox.rect.width, h / symbol->vbox.rect.height);
            cairo_matrix_multiply (&state->affine, &affine, &state->affine);

            cairo_matrix_init_translate (&affine, -symbol->vbox.rect.x, -symbol->vbox.rect.y);
            cairo_matrix_multiply (&state->affine, &affine, &state->affine);

            _hisvg_push_view_box (ctx, symbol->vbox.rect.width, symbol->vbox.rect.height);
            hisvg_push_discrete_layer (ctx);
            if (!state->overflow || (!state->has_overflow && child->state->overflow))
                hisvg_add_clipping_rect (ctx, symbol->vbox.rect.x, symbol->vbox.rect.y,
                                        symbol->vbox.rect.width, symbol->vbox.rect.height);
        } else {
            cairo_matrix_init_translate (&affine, x, y);
            cairo_matrix_multiply (&state->affine, &affine, &state->affine);
            hisvg_push_discrete_layer (ctx);
        }

        hisvg_state_push (ctx);
        _hisvg_node_draw_children (child, ctx, 1);
        hisvg_state_pop (ctx);
        hisvg_pop_discrete_layer (ctx);
        if (symbol->vbox.active)
            _hisvg_pop_view_box (ctx);
    }

    hisvg_release_node (ctx, child);
}

static void
hisvg_node_svg_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGNodeSvg *sself;
    HiSVGState *state;
    cairo_matrix_t affine, affine_old, affine_new;
    guint i;
    double nx, ny, nw, nh;
    sself = (HiSVGNodeSvg *) self;

    nx = _hisvg_css_normalize_length (&sself->x, ctx, 'h');
    ny = _hisvg_css_normalize_length (&sself->y, ctx, 'v');
    nw = _hisvg_css_normalize_length (&sself->w, ctx, 'h');
    nh = _hisvg_css_normalize_length (&sself->h, ctx, 'v');

    hisvg_state_reinherit_top (ctx, self->state, dominate);

    state = hisvg_current_state (ctx);

    affine_old = state->affine;

    if (sself->vbox.active) {
        double x = nx, y = ny, w = nw, h = nh;
        hisvg_preserve_aspect_ratio (sself->preserve_aspect_ratio,
                                    sself->vbox.rect.width, sself->vbox.rect.height,
                                    &w, &h, &x, &y);
        cairo_matrix_init (&affine,
                           w / sself->vbox.rect.width,
                           0,
                           0,
                           h / sself->vbox.rect.height,
                           x - sself->vbox.rect.x * w / sself->vbox.rect.width,
                           y - sself->vbox.rect.y * h / sself->vbox.rect.height);
        cairo_matrix_multiply (&state->affine, &affine, &state->affine);
        _hisvg_push_view_box (ctx, sself->vbox.rect.width, sself->vbox.rect.height);
    } else {
        cairo_matrix_init_translate (&affine, nx, ny);
        cairo_matrix_multiply (&state->affine, &affine, &state->affine);
        _hisvg_push_view_box (ctx, nw, nh);
    }

    affine_new = state->affine;

    hisvg_push_discrete_layer (ctx);

    /* Bounding box addition must be AFTER the discrete layer push, 
       which must be AFTER the transformation happens. */
    if (!state->overflow && HISVG_NODE_HAS_PARENT(self)) {
        state->affine = affine_old;
        hisvg_add_clipping_rect (ctx, nx, ny, nw, nh);
        state->affine = affine_new;
    }

    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(self->base);
    while(child)
    {
        HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        hisvg_state_push (ctx);
        hisvg_node_draw (node, ctx, 0);
        hisvg_state_pop (ctx);
    }

    hisvg_pop_discrete_layer (ctx);
    _hisvg_pop_view_box (ctx);
}

static void
hisvg_node_svg_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *value;
    HiSVGNodeSvg *svg = (HiSVGNodeSvg *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "viewBox")))
        {
            svg->vbox = hisvg_css_parse_vbox (value);
            svg->has_vbox = TRUE;
        }

        if ((value = hisvg_property_bag_lookup (atts, "preserveAspectRatio")))
            svg->preserve_aspect_ratio = hisvg_css_parse_aspect_ratio (value);
        if ((value = hisvg_property_bag_lookup (atts, "width")))
        {
            svg->w = _hisvg_css_parse_length (value);
            svg->has_w = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "height")))
        {
            svg->h = _hisvg_css_parse_length (value);
            svg->has_h = TRUE;
        }
        /* 
         * x & y attributes have no effect on outermost svg
         * http://www.w3.org/TR/SVG/struct.html#SVGElement 
         */
        if (HISVG_NODE_HAS_PARENT(self) && (value = hisvg_property_bag_lookup (atts, "x")))
            svg->x = _hisvg_css_parse_length (value);
        if (HISVG_NODE_HAS_PARENT(self) && (value = hisvg_property_bag_lookup (atts, "y")))
            svg->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            hisvg_defs_register_name (ctx->priv->defs, value, &svg->super);
        }
        /*
         * style element is not loaded yet here, so we need to store those attribues
         * to be applied later.
         */
        svg->atts = hisvg_property_bag_dup(atts);
    }
}

void
_hisvg_node_svg_apply_atts (HiSVGNodeSvg * self, HiSVGHandle * ctx)
{
    const char *id = NULL, *klazz = NULL, *value;
    if (self->atts && hisvg_property_bag_size (self->atts)) {
        if ((value = hisvg_property_bag_lookup (self->atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (self->atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
        }
        hisvg_parse_style_attrs (ctx, ((HiSVGNode *)self)->state, "svg", klazz, id, self->atts);
    }
}

static void
_hisvg_svg_free (HiSVGNode * self)
{
    HiSVGNodeSvg *svg = (HiSVGNodeSvg *) self;

    if (svg->atts) {
        hisvg_property_bag_free (svg->atts);
        svg->atts = NULL;
    }

    _hisvg_node_free (self);
}

HiSVGNode *
hisvg_new_svg (const char* name)
{
    HiSVGNodeSvg *svg;
    svg = g_new (HiSVGNodeSvg, 1);
    _hisvg_node_init (&svg->super, HISVG_NODE_TYPE_SVG, name);
    svg->vbox.active = FALSE;
    svg->preserve_aspect_ratio = HISVG_ASPECT_RATIO_XMID_YMID;
    svg->x = _hisvg_css_parse_length ("0");
    svg->y = _hisvg_css_parse_length ("0");
    svg->w = _hisvg_css_parse_length ("100%");
    svg->h = _hisvg_css_parse_length ("100%");
    svg->super.draw = hisvg_node_svg_draw;
    svg->super.free = _hisvg_svg_free;
    svg->super.set_atts = hisvg_node_svg_set_atts;
    svg->atts = NULL;
    return &svg->super;
}

static void
hisvg_node_use_free (HiSVGNode * node)
{
    HiSVGNodeUse *use = (HiSVGNodeUse *) node;
    g_free (use->link);
    _hisvg_node_free (node);
}

static void
hisvg_node_use_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *value = NULL, *klazz = NULL, *id = NULL;
    HiSVGNodeUse *use;

    use = (HiSVGNodeUse *) self;
    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "x")))
            use->x = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y")))
            use->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "width")))
            use->w = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "height")))
            use->h = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, &use->super);
        }
        if ((value = hisvg_property_bag_lookup (atts, "xlink:href"))) {
            g_free (use->link);
            use->link = g_strdup (value);
        }
        hisvg_parse_style_attrs (ctx, self->state, "use", klazz, id, atts);
    }

}

HiSVGNode *
hisvg_new_use (const char* name)
{
    HiSVGNodeUse *use;
    use = g_new (HiSVGNodeUse, 1);
    _hisvg_node_init (&use->super, HISVG_NODE_TYPE_USE, name);
    use->super.draw = hisvg_node_use_draw;
    use->super.free = hisvg_node_use_free;
    use->super.set_atts = hisvg_node_use_set_atts;
    use->x = _hisvg_css_parse_length ("0");
    use->y = _hisvg_css_parse_length ("0");
    use->w = _hisvg_css_parse_length ("0");
    use->h = _hisvg_css_parse_length ("0");
    use->link = NULL;
    return (HiSVGNode *) use;
}

static void
hisvg_node_symbol_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    HiSVGNodeSymbol *symbol = (HiSVGNodeSymbol *) self;

    const char *klazz = NULL, *value, *id = NULL;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, &symbol->super);
        }
        if ((value = hisvg_property_bag_lookup (atts, "viewBox")))
            symbol->vbox = hisvg_css_parse_vbox (value);
        if ((value = hisvg_property_bag_lookup (atts, "preserveAspectRatio")))
            symbol->preserve_aspect_ratio = hisvg_css_parse_aspect_ratio (value);

        hisvg_parse_style_attrs (ctx, self->state, "symbol", klazz, id, atts);
    }

}


HiSVGNode *
hisvg_new_symbol (const char* name)
{
    HiSVGNodeSymbol *symbol;
    symbol = g_new (HiSVGNodeSymbol, 1);
    _hisvg_node_init (&symbol->super, HISVG_NODE_TYPE_SYMBOL, name);
    symbol->vbox.active = FALSE;
    symbol->preserve_aspect_ratio = HISVG_ASPECT_RATIO_XMID_YMID;
    symbol->super.draw = _hisvg_node_draw_nothing;
    symbol->super.set_atts = hisvg_node_symbol_set_atts;
    return &symbol->super;
}

HiSVGNode *
hisvg_new_defs (const char* name)
{
    HiSVGNodeGroup *group;
    group = g_new (HiSVGNodeGroup, 1);
    _hisvg_node_init (&group->super, HISVG_NODE_TYPE_DEFS, name);
    group->super.draw = _hisvg_node_draw_nothing;
    group->super.set_atts = hisvg_node_group_set_atts;
    return &group->super;
}

static void
_hisvg_node_switch_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    guint i;

    hisvg_state_reinherit_top (ctx, self->state, dominate);

    hisvg_push_discrete_layer (ctx);

    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(self->base);
    while(child)
    {
        HiSVGNode *drawable = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        if (drawable->state->cond_true) {
            hisvg_state_push (ctx);
            hisvg_node_draw (drawable, ctx, 0);
            hisvg_state_pop (ctx);

            break;              /* only render the 1st one */
        }
    }

    hisvg_pop_discrete_layer (ctx);
}

HiSVGNode *
hisvg_new_switch (const char* name)
{
    HiSVGNodeGroup *group;
    group = g_new (HiSVGNodeGroup, 1);
    _hisvg_node_init (&group->super, HISVG_NODE_TYPE_SWITCH, name);
    group->super.draw = _hisvg_node_switch_draw;
    group->super.set_atts = hisvg_node_group_set_atts;
    return &group->super;
}
