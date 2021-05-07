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

#include "hisvg-private.h"
#include "hisvg-defs.h"
#include "hisvg-paint-server.h"
#include "hisvg-styles.h"
#include "hisvg-image.h"

#include <glib.h>
#include <string.h>
#include <math.h>

#include "hisvg-css.h"

HiSVGPaintServer *
hisvg_paint_server_solid (guint32 argb)
{
    HiSVGPaintServer *result = g_new (HiSVGPaintServer, 1);

    result->refcnt = 1;
    result->type = HISVG_PAINT_SERVER_SOLID;
    result->core.color = g_new (HiSVGSolidColor, 1);
    result->core.color->argb = argb;
    result->core.color->currentcolor = FALSE;

    return result;
}

HiSVGPaintServer *
hisvg_paint_server_solid_current_color (void)
{
    HiSVGPaintServer *result = g_new (HiSVGPaintServer, 1);

    result->refcnt = 1;
    result->type = HISVG_PAINT_SERVER_SOLID;
    result->core.color = g_new (HiSVGSolidColor, 1);
    result->core.color->currentcolor = TRUE;

    return result;
}

HiSVGPaintServer *
hisvg_paint_server_iri (char *iri)
{
    HiSVGPaintServer *result = g_new (HiSVGPaintServer, 1);

    result->refcnt = 1;
    result->type = HISVG_PAINT_SERVER_IRI;
    result->core.iri = iri;

    return result;
}

/**
 * hisvg_paint_server_parse:
 * @str: The SVG paint specification string to parse.
 *
 * Parses the paint specification @str, creating a new paint server
 * object.
 *
 * Return value: (nullable): The newly created paint server, or %NULL
 *   on error.
 **/
HiSVGPaintServer *
hisvg_paint_server_parse (gboolean * inherit, const char *str)
{
    char *name;
    guint32 argb;
    if (inherit != NULL)
        *inherit = 1;
    if (str == NULL || !strcmp (str, "none"))
        return NULL;

    name = hisvg_get_url_string (str);
    if (name) {
        return hisvg_paint_server_iri (name);
    } else if (!strcmp (str, "inherit")) {
        if (inherit != NULL)
            *inherit = 0;
        return hisvg_paint_server_solid (0);
    } else if (!strcmp (str, "currentColor")) {
        HiSVGPaintServer *ps;
        ps = hisvg_paint_server_solid_current_color ();
        return ps;
    } else {
        argb = hisvg_css_parse_color (str, inherit);
        return hisvg_paint_server_solid (argb);
    }
}

/**
 * hisvg_paint_server_ref:
 * @ps: The paint server object to reference.
 *
 * Reference a paint server object.
 **/
void
hisvg_paint_server_ref (HiSVGPaintServer * ps)
{
    if (ps == NULL)
        return;
    ps->refcnt++;
}

/**
 * hisvg_paint_server_unref:
 * @ps: The paint server object to unreference.
 *
 * Unreference a paint server object.
 **/
void
hisvg_paint_server_unref (HiSVGPaintServer * ps)
{
    if (ps == NULL)
        return;
    if (--ps->refcnt == 0) {
        if (ps->type == HISVG_PAINT_SERVER_SOLID)
            g_free (ps->core.color);
        else if (ps->type == HISVG_PAINT_SERVER_IRI)
            g_free (ps->core.iri);
        g_free (ps);
    }
}

static void
hisvg_stop_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    double offset = 0;
    gboolean is_current_color = FALSE;
    const char *value;
    HiSVGGradientStop *stop;
    HiSVGState state;

    stop = (HiSVGGradientStop *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "offset"))) {
            /* either a number [0,1] or a percentage */
            HiSVGLength length = _hisvg_css_parse_length (value);
            offset = _hisvg_css_hand_normalize_length (&length, hisvg_dpi_percentage (ctx), 1., 0.);

            if (offset < 0.)
                offset = 0.;
            else if (offset > 1.)
                offset = 1.;
            stop->offset = offset;
        }
        if ((value = hisvg_property_bag_lookup (atts, "style")))
            hisvg_parse_style (ctx, self->state, value);

        if ((value = hisvg_property_bag_lookup (atts, "stop-color")))
            if (!strcmp (value, "currentColor"))
                is_current_color = TRUE;

        hisvg_parse_style_pairs (ctx, self->state, atts);
    }
    // FIXME: xue 20200417 remove node.parent
//    self->parent = ctx->priv->currentnode;
    hisvg_state_init (&state);
    hisvg_state_reconstruct (&state, self);
    if (is_current_color)
        state.stop_color = state.current_color;
    stop->rgba = (state.stop_color << 8) | state.stop_opacity;
    hisvg_state_finalize (&state);
}

HiSVGNode *
hisvg_new_stop (const char* name)
{
    HiSVGGradientStop *stop = g_new (HiSVGGradientStop, 1);
    _hisvg_node_init (&stop->super, HISVG_NODE_TYPE_STOP, name);
    stop->super.set_atts = hisvg_stop_set_atts;
    stop->offset = 0;
    stop->rgba = 0;
    return &stop->super;
}

static void
hisvg_linear_gradient_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    HiSVGLinearGradient *grad = (HiSVGLinearGradient *) self;
    const char *value;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }
        if ((value = hisvg_property_bag_lookup (atts, "x1"))) {
            grad->x1 = _hisvg_css_parse_length (value);
            grad->hasx1 = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "y1"))) {
            grad->y1 = _hisvg_css_parse_length (value);
            grad->hasy1 = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "x2"))) {
            grad->x2 = _hisvg_css_parse_length (value);
            grad->hasx2 = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "y2"))) {
            grad->y2 = _hisvg_css_parse_length (value);
            grad->hasy2 = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "spreadMethod"))) {
            if (!strcmp (value, "pad")) {
                grad->spread = CAIRO_EXTEND_PAD;
            } else if (!strcmp (value, "reflect")) {
                grad->spread = CAIRO_EXTEND_REFLECT;
            } else if (!strcmp (value, "repeat")) {
                grad->spread = CAIRO_EXTEND_REPEAT;
            }
            grad->hasspread = TRUE;
        }
        g_free (grad->fallback);
        grad->fallback = g_strdup (hisvg_property_bag_lookup (atts, "xlink:href"));
        if ((value = hisvg_property_bag_lookup (atts, "gradientTransform"))) {
            hisvg_parse_transform (&grad->affine, value);
            grad->hastransform = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "color")))
            grad->current_color = hisvg_css_parse_color (value, 0);
        if ((value = hisvg_property_bag_lookup (atts, "gradientUnits"))) {
            if (!strcmp (value, "userSpaceOnUse"))
                grad->obj_bbox = FALSE;
            else if (!strcmp (value, "objectBoundingBox"))
                grad->obj_bbox = TRUE;
            grad->hasbbox = TRUE;
        }
        hisvg_parse_style_attrs (ctx, self->state, "linearGradient", NULL, NULL, atts);
    }
}

static void
hisvg_linear_gradient_free (HiSVGNode * node)
{
    HiSVGLinearGradient *self = (HiSVGLinearGradient *) node;
    g_free (self->fallback);
    _hisvg_node_free (node);
}

HiSVGNode *
hisvg_new_linear_gradient (const char* name)
{
    HiSVGLinearGradient *grad = NULL;
    grad = g_new (HiSVGLinearGradient, 1);
    _hisvg_node_init (&grad->super, HISVG_NODE_TYPE_LINEAR_GRADIENT, name);
    cairo_matrix_init_identity (&grad->affine);
    grad->has_current_color = FALSE;
    grad->x1 = grad->y1 = grad->y2 = _hisvg_css_parse_length ("0");
    grad->x2 = _hisvg_css_parse_length ("1");
    grad->fallback = NULL;
    grad->obj_bbox = TRUE;
    grad->spread = CAIRO_EXTEND_PAD;
    grad->super.free = hisvg_linear_gradient_free;
    grad->super.set_atts = hisvg_linear_gradient_set_atts;
    grad->hasx1 = grad->hasy1 = grad->hasx2 = grad->hasy2 = grad->hasbbox = grad->hasspread =
        grad->hastransform = FALSE;
    return &grad->super;
}

static void
hisvg_radial_gradient_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    HiSVGRadialGradient *grad = (HiSVGRadialGradient *) self;
    const char *value;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }
        if ((value = hisvg_property_bag_lookup (atts, "cx"))) {
            grad->cx = _hisvg_css_parse_length (value);
            grad->hascx = TRUE;
            if (!grad->hasfx)
                grad->fx = grad->cx;
        }
        if ((value = hisvg_property_bag_lookup (atts, "cy"))) {
            grad->cy = _hisvg_css_parse_length (value);
            grad->hascy = TRUE;
            if (!grad->hasfy)
                grad->fy = grad->cy;
        }
        if ((value = hisvg_property_bag_lookup (atts, "r"))) {
            grad->r = _hisvg_css_parse_length (value);
            grad->hasr = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "fx"))) {
            grad->fx = _hisvg_css_parse_length (value);
            grad->hasfx = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "fy"))) {
            grad->fy = _hisvg_css_parse_length (value);
            grad->hasfy = TRUE;
        }
        g_free (grad->fallback);
        grad->fallback = g_strdup (hisvg_property_bag_lookup (atts, "xlink:href"));
        if ((value = hisvg_property_bag_lookup (atts, "gradientTransform"))) {
            hisvg_parse_transform (&grad->affine, value);
            grad->hastransform = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "color"))) {
            grad->current_color = hisvg_css_parse_color (value, 0);
        }
        if ((value = hisvg_property_bag_lookup (atts, "spreadMethod"))) {
            if (!strcmp (value, "pad"))
                grad->spread = CAIRO_EXTEND_PAD;
            else if (!strcmp (value, "reflect"))
                grad->spread = CAIRO_EXTEND_REFLECT;
            else if (!strcmp (value, "repeat"))
                grad->spread = CAIRO_EXTEND_REPEAT;
            grad->hasspread = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "gradientUnits"))) {
            if (!strcmp (value, "userSpaceOnUse"))
                grad->obj_bbox = FALSE;
            else if (!strcmp (value, "objectBoundingBox"))
                grad->obj_bbox = TRUE;
            grad->hasbbox = TRUE;
        }
        hisvg_parse_style_attrs (ctx, self->state, "radialGradient", NULL, NULL, atts);
    }
}

static void
hisvg_radial_gradient_free (HiSVGNode * node)
{
    HiSVGRadialGradient *self = (HiSVGRadialGradient *) node;
    g_free (self->fallback);
    _hisvg_node_free (node);
}

HiSVGNode *
hisvg_new_radial_gradient (const char* name)
{

    HiSVGRadialGradient *grad = g_new (HiSVGRadialGradient, 1);
    _hisvg_node_init (&grad->super, HISVG_NODE_TYPE_RADIAL_GRADIENT, name);
    cairo_matrix_init_identity (&grad->affine);
    grad->has_current_color = FALSE;
    grad->obj_bbox = TRUE;
    grad->spread = CAIRO_EXTEND_PAD;
    grad->fallback = NULL;
    grad->cx = grad->cy = grad->r = grad->fx = grad->fy = _hisvg_css_parse_length ("0.5");
    grad->super.free = hisvg_radial_gradient_free;
    grad->super.set_atts = hisvg_radial_gradient_set_atts;
    grad->hascx = grad->hascy = grad->hasfx = grad->hasfy = grad->hasr = grad->hasbbox =
        grad->hasspread = grad->hastransform = FALSE;
    return &grad->super;
}

static void
hisvg_pattern_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    HiSVGPattern *pattern = (HiSVGPattern *) self;
    const char *value;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }
        if ((value = hisvg_property_bag_lookup (atts, "viewBox"))) {
            pattern->vbox = hisvg_css_parse_vbox (value);
            pattern->hasvbox = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "x"))) {
            pattern->x = _hisvg_css_parse_length (value);
            pattern->hasx = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "y"))) {
            pattern->y = _hisvg_css_parse_length (value);
            pattern->hasy = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "width"))) {
            pattern->width = _hisvg_css_parse_length (value);
            pattern->haswidth = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "height"))) {
            pattern->height = _hisvg_css_parse_length (value);
            pattern->hasheight = TRUE;
        }
        g_free (pattern->fallback);
        pattern->fallback = g_strdup (hisvg_property_bag_lookup (atts, "xlink:href"));
        if ((value = hisvg_property_bag_lookup (atts, "patternTransform"))) {
            hisvg_parse_transform (&pattern->affine, value);
            pattern->hastransform = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "patternUnits"))) {
            if (!strcmp (value, "userSpaceOnUse"))
                pattern->obj_bbox = FALSE;
            else if (!strcmp (value, "objectBoundingBox"))
                pattern->obj_bbox = TRUE;
            pattern->hasbbox = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "patternContentUnits"))) {
            if (!strcmp (value, "userSpaceOnUse"))
                pattern->obj_cbbox = FALSE;
            else if (!strcmp (value, "objectBoundingBox"))
                pattern->obj_cbbox = TRUE;
            pattern->hascbox = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "preserveAspectRatio"))) {
            pattern->preserve_aspect_ratio = hisvg_css_parse_aspect_ratio (value);
            pattern->hasaspect = TRUE;
        }
    }
}

static void
hisvg_pattern_free (HiSVGNode * node)
{
    HiSVGPattern *self = (HiSVGPattern *) node;
    g_free (self->fallback);
    _hisvg_node_free (node);
}


HiSVGNode *
hisvg_new_pattern (const char* name)
{
    HiSVGPattern *pattern = g_new (HiSVGPattern, 1);
    _hisvg_node_init (&pattern->super, HISVG_NODE_TYPE_PATTERN, name);
    cairo_matrix_init_identity (&pattern->affine);
    pattern->obj_bbox = TRUE;
    pattern->obj_cbbox = FALSE;
    pattern->x = pattern->y = pattern->width = pattern->height = _hisvg_css_parse_length ("0");
    pattern->fallback = NULL;
    pattern->preserve_aspect_ratio = HISVG_ASPECT_RATIO_XMID_YMID;
    pattern->vbox.active = FALSE;
    pattern->super.free = hisvg_pattern_free;
    pattern->super.set_atts = hisvg_pattern_set_atts;
    pattern->hasx = pattern->hasy = pattern->haswidth = pattern->hasheight = pattern->hasbbox =
        pattern->hascbox = pattern->hasvbox = pattern->hasaspect = pattern->hastransform = FALSE;
    return &pattern->super;
}

static int
hasstop_child (HiSVGNode* n)
{
    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(n->base);
    while(child)
    {
        HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_STOP)
            return 1;
    }
    return 0;
}

typedef const char * (* GetFallbackFn) (HiSVGNode *node);
typedef void (* ApplyFallbackFn) (HiSVGNode *node, HiSVGNode *fallback_node);

/* Some SVG paint servers can reference a "parent" or "fallback" paint server
 * through the xlink:href attribute (for example,
 * http://www.w3.org/TR/SVG11/pservers.html#LinearGradientElementHrefAttribute )
 * This is used to define a chain of properties to be resolved from each
 * fallback.
 */
static void
resolve_fallbacks (HiSVGDrawingCtx *ctx,
                   HiSVGNode *data,
                   HiSVGNode *last_fallback,
                   GetFallbackFn get_fallback,
                   ApplyFallbackFn apply_fallback)
{
    HiSVGNode *fallback;
    const char *fallback_id;

    fallback_id = get_fallback (last_fallback);
    if (fallback_id == NULL)
        return;
    fallback = hisvg_acquire_node (ctx, fallback_id);
    if (fallback == NULL)
      return;

    apply_fallback (data, fallback);
    resolve_fallbacks (ctx, data, fallback, get_fallback, apply_fallback);

    hisvg_release_node (ctx, fallback);
}

static const char *
gradient_get_fallback (HiSVGNode *node)
{
    if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_LINEAR_GRADIENT) {
        HiSVGLinearGradient *g = (HiSVGLinearGradient *) node;
        return g->fallback;
    } else if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_RADIAL_GRADIENT) {
        HiSVGRadialGradient *g = (HiSVGRadialGradient *) node;
        return g->fallback;
    } else
        return NULL;
}

static void _apply_children(HiSVGNode* dst, HiSVGNode* src)
{
    if (dst == NULL || src == NULL)
    {
        return;
    }

    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(src->base);
    while(child)
    {
        HLDomElementNode* element = child;
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        HISVG_DOM_ELEMENT_NODE_ADD_CHILD(dst->base, element);
    }
}

static void
linear_gradient_apply_fallback (HiSVGNode *node, HiSVGNode *fallback_node)
{
    HiSVGLinearGradient *grad;

    g_assert (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_LINEAR_GRADIENT);
    grad = (HiSVGLinearGradient *) node;

    if (HISVG_NODE_TYPE (fallback_node) == HISVG_NODE_TYPE_LINEAR_GRADIENT) {
        HiSVGLinearGradient *fallback = (HiSVGLinearGradient *) fallback_node;

        if (!grad->hasx1 && fallback->hasx1) {
            grad->hasx1 = TRUE;
            grad->x1 = fallback->x1;
        }
        if (!grad->hasy1 && fallback->hasy1) {
            grad->hasy1 = TRUE;
            grad->y1 = fallback->y1;
        }
        if (!grad->hasx2 && fallback->hasx2) {
            grad->hasx2 = TRUE;
            grad->x2 = fallback->x2;
        }
        if (!grad->hasy2 && fallback->hasy2) {
            grad->hasy2 = TRUE;
            grad->y2 = fallback->y2;
        }
        if (!grad->hastransform && fallback->hastransform) {
            grad->hastransform = TRUE;
            grad->affine = fallback->affine;
        }
        if (!grad->hasspread && fallback->hasspread) {
            grad->hasspread = TRUE;
            grad->spread = fallback->spread;
        }
        if (!grad->hasbbox && fallback->hasbbox) {
            grad->hasbbox = TRUE;
            grad->obj_bbox = fallback->obj_bbox;
        }
        if (!hasstop_child ((HiSVGNode*)grad) && hasstop_child ((HiSVGNode*)fallback)) {
            _apply_children((HiSVGNode*)grad, (HiSVGNode*)fallback);
        }
    } else if (HISVG_NODE_TYPE (fallback_node) == HISVG_NODE_TYPE_RADIAL_GRADIENT) {
        HiSVGRadialGradient *fallback = (HiSVGRadialGradient *) fallback_node;

        if (!grad->hastransform && fallback->hastransform) {
            grad->hastransform = TRUE;
            grad->affine = fallback->affine;
        }
        if (!grad->hasspread && fallback->hasspread) {
            grad->hasspread = TRUE;
            grad->spread = fallback->spread;
        }
        if (!grad->hasbbox && fallback->hasbbox) {
            grad->hasbbox = TRUE;
            grad->obj_bbox = fallback->obj_bbox;
        }
        if (!hasstop_child ((HiSVGNode*)grad) && hasstop_child ((HiSVGNode*)fallback)) {
            _apply_children((HiSVGNode*)grad, (HiSVGNode*)fallback);
        }
    }
}

void
hisvg_linear_gradient_fix_fallback (HiSVGDrawingCtx *ctx, HiSVGLinearGradient * grad)
{
    resolve_fallbacks (ctx,
                       (HiSVGNode *) grad,
                       (HiSVGNode *) grad,
                       gradient_get_fallback,
                       linear_gradient_apply_fallback);
}

static void
radial_gradient_apply_fallback (HiSVGNode *node, HiSVGNode *fallback_node)
{
    HiSVGRadialGradient *grad;

    g_assert (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_RADIAL_GRADIENT);
    grad = (HiSVGRadialGradient *) node;

    if (HISVG_NODE_TYPE (fallback_node) == HISVG_NODE_TYPE_RADIAL_GRADIENT) {
        HiSVGRadialGradient *fallback = (HiSVGRadialGradient *) fallback_node;

        if (!grad->hascx && fallback->hascx) {
            grad->hascx = TRUE;
            grad->cx = fallback->cx;
        }
        if (!grad->hascy && fallback->hascy) {
            grad->hascy = TRUE;
            grad->cy = fallback->cy;
        }
        if (!grad->hasfx && fallback->hasfx) {
            grad->hasfx = TRUE;
            grad->fx = fallback->fx;
        }
        if (!grad->hasfy && fallback->hasfy) {
            grad->hasfy = TRUE;
            grad->fy = fallback->fy;
        }
        if (!grad->hasr && fallback->hasr) {
            grad->hasr = TRUE;
            grad->r = fallback->r;
        }
        if (!grad->hastransform && fallback->hastransform) {
            grad->hastransform = TRUE;
            grad->affine = fallback->affine;
        }
        if (!grad->hasspread && fallback->hasspread) {
            grad->hasspread = TRUE;
            grad->spread = fallback->spread;
        }
        if (!grad->hasbbox && fallback->hasbbox) {
            grad->hasbbox = TRUE;
            grad->obj_bbox = fallback->obj_bbox;
        }
        if (!hasstop_child ((HiSVGNode*)grad) && hasstop_child ((HiSVGNode*)fallback)) {
            _apply_children((HiSVGNode*)grad, (HiSVGNode*)fallback);
        }
    } else if (HISVG_NODE_TYPE (fallback_node) == HISVG_NODE_TYPE_LINEAR_GRADIENT) {
        HiSVGLinearGradient *fallback = (HiSVGLinearGradient *) fallback_node;

        if (!grad->hastransform && fallback->hastransform) {
            grad->hastransform = TRUE;
            grad->affine = fallback->affine;
        }
        if (!grad->hasspread && fallback->hasspread) {
            grad->hasspread = TRUE;
            grad->spread = fallback->spread;
        }
        if (!grad->hasbbox && fallback->hasbbox) {
            grad->hasbbox = TRUE;
            grad->obj_bbox = fallback->obj_bbox;
        }
        if (!hasstop_child ((HiSVGNode*)grad) && hasstop_child ((HiSVGNode*)fallback)) {
            _apply_children((HiSVGNode*)grad, (HiSVGNode*)fallback);
        }
    }
}

void
hisvg_radial_gradient_fix_fallback (HiSVGDrawingCtx *ctx, HiSVGRadialGradient * grad)
{
    resolve_fallbacks (ctx,
                       (HiSVGNode *) grad,
                       (HiSVGNode *) grad,
                       gradient_get_fallback,
                       radial_gradient_apply_fallback);
}

static const char *
pattern_get_fallback (HiSVGNode *node)
{
    if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_PATTERN) {
        HiSVGPattern *pattern = (HiSVGPattern *) node;

        return pattern->fallback;
    } else
        return NULL;
}

static void
pattern_apply_fallback (HiSVGNode *pattern_node, HiSVGNode *fallback_node)
{
    HiSVGPattern *pattern;
    HiSVGPattern *fallback;

    g_assert (HISVG_NODE_TYPE (pattern_node) == HISVG_NODE_TYPE_PATTERN);

    if (HISVG_NODE_TYPE (fallback_node) != HISVG_NODE_TYPE_PATTERN)
        return;

    pattern = (HiSVGPattern *) pattern_node;
    fallback = (HiSVGPattern *) fallback_node;

    if (!pattern->hasx && fallback->hasx) {
        pattern->hasx = TRUE;
        pattern->x = fallback->x;
    }
    if (!pattern->hasy && fallback->hasy) {
        pattern->hasy = TRUE;
        pattern->y = fallback->y;
    }
    if (!pattern->haswidth && fallback->haswidth) {
        pattern->haswidth = TRUE;
        pattern->width = fallback->width;
    }
    if (!pattern->hasheight && fallback->hasheight) {
        pattern->hasheight = TRUE;
        pattern->height = fallback->height;
    }
    if (!pattern->hastransform && fallback->hastransform) {
        pattern->hastransform = TRUE;
        pattern->affine = fallback->affine;
    }
    if (!pattern->hasvbox && fallback->hasvbox) {
        pattern->vbox = fallback->vbox;
    }
    if (!pattern->hasaspect && fallback->hasaspect) {
        pattern->hasaspect = TRUE;
        pattern->preserve_aspect_ratio = fallback->preserve_aspect_ratio;
    }
    if (!pattern->hasbbox && fallback->hasbbox) {
        pattern->hasbbox = TRUE;
        pattern->obj_bbox = fallback->obj_bbox;
    }
    if (!pattern->hascbox && fallback->hascbox) {
        pattern->hascbox = TRUE;
        pattern->obj_cbbox = fallback->obj_cbbox;
    }
    if (!HISVG_NODE_CHILDREN_COUNT(((HiSVGNode*)pattern)) && HISVG_NODE_CHILDREN_COUNT(((HiSVGNode*)fallback))) {
        _apply_children((HiSVGNode*)pattern, (HiSVGNode*)fallback);
    }
}

void
hisvg_pattern_fix_fallback (HiSVGDrawingCtx *ctx, HiSVGPattern * pattern)
{
    resolve_fallbacks (ctx,
                       (HiSVGNode *) pattern,
                       (HiSVGNode *) pattern,
                       pattern_get_fallback,
                       pattern_apply_fallback);
}
