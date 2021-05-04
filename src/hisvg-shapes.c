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
#include <string.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>

#include "hisvg-private.h"
#include "hisvg-styles.h"
#include "hisvg-shapes.h"
#include "hisvg-css.h"
#include "hisvg-defs.h"
#include "hisvg-path.h"

/* 4/3 * (1-cos 45)/sin 45 = 4/3 * sqrt(2) - 1 */
#define HISVG_ARC_MAGIC ((double) 0.5522847498)

static void
hisvg_node_path_free (HiSVGNode * self)
{
    HiSVGNodePath *path = (HiSVGNodePath *) self;
    if (path->path)
        hisvg_cairo_path_destroy (path->path);
    _hisvg_node_finalize (&path->super);
    g_free (path);
}

static void
hisvg_node_path_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGNodePath *path = (HiSVGNodePath *) self;

    if (!path->path)
        return;

    hisvg_state_reinherit_top (ctx, self->state, dominate);

    hisvg_render_path (ctx, path->path);
}

static void
hisvg_node_path_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodePath *path = (HiSVGNodePath *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "d"))) {
            if (path->path)
                hisvg_cairo_path_destroy (path->path);
            path->path = hisvg_parse_path (value);
        }
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "path", klazz, id, atts);
    }
}

HiSVGNode *
hisvg_new_path (const char* name)
{
    HiSVGNodePath *path;
    path = g_new (HiSVGNodePath, 1);
    _hisvg_node_init (&path->super, HISVG_NODE_TYPE_PATH, name);
    path->path = NULL;
    path->super.free = hisvg_node_path_free;
    path->super.draw = hisvg_node_path_draw;
    path->super.set_atts = hisvg_node_path_set_atts;

    return &path->super;
}

struct _HiSVGNodePoly {
    HiSVGNode super;
    cairo_path_t *path;
};

typedef struct _HiSVGNodePoly HiSVGNodePoly;

static cairo_path_t *
_hisvg_node_poly_build_path (const char *value,
                            gboolean close_path);

static void
_hisvg_node_poly_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    HiSVGNodePoly *poly = (HiSVGNodePoly *) self;
    const char *klazz = NULL, *id = NULL, *value;

    if (hisvg_property_bag_size (atts)) {
        /* support for svg < 1.0 which used verts */
        if ((value = hisvg_property_bag_lookup (atts, "verts"))
            || (value = hisvg_property_bag_lookup (atts, "points"))) {
            if (poly->path)
                hisvg_cairo_path_destroy (poly->path);
            poly->path = _hisvg_node_poly_build_path (value,
                                                     HISVG_NODE_TYPE (self) == HISVG_NODE_TYPE_POLYGON);
        }
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state,
                                HISVG_NODE_TYPE (self) == HISVG_NODE_TYPE_POLYLINE ? "polyline" : "polygon",
                                klazz, id, atts);
    }

}

static cairo_path_t *
_hisvg_node_poly_build_path (const char *value,
                            gboolean close_path)
{
    double *pointlist;
    guint pointlist_len, i;
    HiSVGPathBuilder builder;
    cairo_path_t *path;

    pointlist = hisvg_css_parse_number_list (value, &pointlist_len);
    if (pointlist == NULL)
        return NULL;

    if (pointlist_len < 2) {
        g_free (pointlist);
        return NULL;
    }

    /* Calculate the number of cairo_path_data_t we'll need:
     *
     *     pointlist_len / 2 -> number of commands
     *     pointlist_len / 2 -> number of points
     * +   1                 -> closepath
     * ---------------------------------------------
     *     pointlist_len + 1 -> total
     */
    hisvg_path_builder_init (&builder, pointlist_len + 1);

    hisvg_path_builder_move_to (&builder, pointlist[0], pointlist[1]);

    for (i = 2; i < pointlist_len; i += 2) {
        double x, y;

        x = pointlist[i];

        /* We expect points to come in coordinate pairs.  But if there is a
         * missing part of one pair in a corrupt SVG, we'll have an incomplete
         * list.  In that case, we reuse the last-known Y coordinate.
         */
        if (i + 1 < pointlist_len)
            y = pointlist[i + 1];
        else
            y = pointlist[i - 1];

        hisvg_path_builder_line_to (&builder, x, y);
    }

    if (close_path)
        hisvg_path_builder_close_path (&builder);

    path = hisvg_path_builder_finish (&builder);
    g_free (pointlist);

    return path;
}

static void
_hisvg_node_poly_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGNodePoly *poly = (HiSVGNodePoly *) self;

    if (poly->path == NULL)
        return;

    hisvg_state_reinherit_top (ctx, self->state, dominate);

    hisvg_render_path (ctx, poly->path);
}

static void
_hisvg_node_poly_free (HiSVGNode * self)
{
    HiSVGNodePoly *poly = (HiSVGNodePoly *) self;
    if (poly->path)
        hisvg_cairo_path_destroy (poly->path);
    _hisvg_node_finalize (&poly->super);
    g_free (poly);
}

static HiSVGNode *
hisvg_new_any_poly (HiSVGNodeType type, const char* name)
{
    HiSVGNodePoly *poly;
    poly = g_new (HiSVGNodePoly, 1);
    _hisvg_node_init (&poly->super, type, name);
    poly->super.free = _hisvg_node_poly_free;
    poly->super.draw = _hisvg_node_poly_draw;
    poly->super.set_atts = _hisvg_node_poly_set_atts;
    poly->path = NULL;
    return &poly->super;
}

HiSVGNode *
hisvg_new_polygon (const char* name)
{
    return hisvg_new_any_poly (HISVG_NODE_TYPE_POLYGON, name);
}

HiSVGNode *
hisvg_new_polyline (const char* name)
{
    return hisvg_new_any_poly (HISVG_NODE_TYPE_POLYLINE, name);
}


struct _HiSVGNodeLine {
    HiSVGNode super;
    HiSVGLength x1, x2, y1, y2;
};

typedef struct _HiSVGNodeLine HiSVGNodeLine;

static void
_hisvg_node_line_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeLine *line = (HiSVGNodeLine *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "x1")))
            line->x1 = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y1")))
            line->y1 = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "x2")))
            line->x2 = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y2")))
            line->y2 = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "line", klazz, id, atts);
    }
}

static void
_hisvg_node_line_draw (HiSVGNode * overself, HiSVGDrawingCtx * ctx, int dominate)
{
    cairo_path_t *path;
    HiSVGPathBuilder builder;
    HiSVGNodeLine *self = (HiSVGNodeLine *) overself;
    double x1, y1, x2, y2;

    hisvg_path_builder_init (&builder, 4);

    x1 = _hisvg_css_normalize_length (&self->x1, ctx, 'h');
    y1 = _hisvg_css_normalize_length (&self->y1, ctx, 'v');
    x2 = _hisvg_css_normalize_length (&self->x2, ctx, 'h');
    y2 = _hisvg_css_normalize_length (&self->y2, ctx, 'v');

    hisvg_path_builder_move_to (&builder, x1, y1);
    hisvg_path_builder_line_to (&builder, x2, y2);

    path = hisvg_path_builder_finish (&builder);

    hisvg_state_reinherit_top (ctx, overself->state, dominate);

    hisvg_render_path (ctx, path);
    hisvg_cairo_path_destroy (path);
}

HiSVGNode *
hisvg_new_line (const char* name)
{
    HiSVGNodeLine *line;
    line = g_new (HiSVGNodeLine, 1);
    _hisvg_node_init (&line->super, HISVG_NODE_TYPE_LINE, name);
    line->super.draw = _hisvg_node_line_draw;
    line->super.set_atts = _hisvg_node_line_set_atts;
    line->x1 = line->x2 = line->y1 = line->y2 = _hisvg_css_parse_length ("0");
    return &line->super;
}

struct _HiSVGNodeRect {
    HiSVGNode super;
    HiSVGLength x, y, w, h, rx, ry;
    gboolean got_rx, got_ry;
};

typedef struct _HiSVGNodeRect HiSVGNodeRect;

static void
_hisvg_node_rect_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeRect *rect = (HiSVGNodeRect *) self;

    /* FIXME: negative w/h/rx/ry is an error, per http://www.w3.org/TR/SVG11/shapes.html#RectElement */
    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "x")))
            rect->x = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y")))
            rect->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "width")))
            rect->w = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "height")))
            rect->h = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "rx"))) {
            rect->rx = _hisvg_css_parse_length (value);
            rect->got_rx = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "ry"))) {
            rect->ry = _hisvg_css_parse_length (value);
            rect->got_ry = TRUE;
        }
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "rect", klazz, id, atts);
    }
}

static void
_hisvg_node_rect_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    double x, y, w, h, rx, ry;
    double half_w, half_h;
    HiSVGPathBuilder builder;
    cairo_path_t *path;
    HiSVGNodeRect *rect = (HiSVGNodeRect *) self;

    x = _hisvg_css_normalize_length (&rect->x, ctx, 'h');
    y = _hisvg_css_normalize_length (&rect->y, ctx, 'v');

    /* FIXME: negative w/h/rx/ry is an error, per http://www.w3.org/TR/SVG11/shapes.html#RectElement
     * For now we'll just take the absolute value.
     */
    w = fabs (_hisvg_css_normalize_length (&rect->w, ctx, 'h'));
    h = fabs (_hisvg_css_normalize_length (&rect->h, ctx, 'v'));
    rx = fabs (_hisvg_css_normalize_length (&rect->rx, ctx, 'h'));
    ry = fabs (_hisvg_css_normalize_length (&rect->ry, ctx, 'v'));

    if (w == 0. || h == 0.)
        return;

    if (rect->got_rx)
        rx = rx;
    else
        rx = ry;

    if (rect->got_ry)
        ry = ry;
    else
        ry = rx;

    half_w = w / 2;
    half_h = h / 2;

    if (rx > half_w)
        rx = half_w;

    if (ry > half_h)
        ry = half_h;

    if (rx == 0)
        ry = 0;
    else if (ry == 0)
        rx = 0;

    if (rx == 0) {
        /* Easy case, no rounded corners */

        hisvg_path_builder_init (&builder, 11);

        hisvg_path_builder_move_to (&builder, x, y);
        hisvg_path_builder_line_to (&builder, x + w, y);
        hisvg_path_builder_line_to (&builder, x + w, y + h);
        hisvg_path_builder_line_to (&builder, x, y + h);
        hisvg_path_builder_line_to (&builder, x, y);
        hisvg_path_builder_close_path (&builder);
    } else {
        double top_x1, top_x2, top_y;
        double bottom_x1, bottom_x2, bottom_y;
        double left_x, left_y1, left_y2;
        double right_x, right_y1, right_y2;

        /* Hard case, rounded corners
         *
         *      (top_x1, top_y)                   (top_x2, top_y)
         *     *--------------------------------*
         *    /                                  \
         *   * (left_x, left_y1)                  * (right_x, right_y1)
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   |                                    |
         *   * (left_x, left_y2)                  * (right_x, right_y2)
         *    \                                  /
         *     *--------------------------------*
         *      (bottom_x1, bottom_y)            (bottom_x2, bottom_y)
         */

        top_x1 = x + rx;
        top_x2 = x + w - rx;
        top_y  = y;

        bottom_x1 = top_x1;
        bottom_x2 = top_x2;
        bottom_y  = y + h;

        left_x = x;
        left_y1 = y + ry;
        left_y2 = y + h - ry;

        right_x = x + w;
        right_y1 = left_y1;
        right_y2 = left_y2;

        hisvg_path_builder_init (&builder, 32); /* an estimate; the arc segments may grow the array anyway */

        hisvg_path_builder_move_to (&builder, top_x1, top_y);
        hisvg_path_builder_line_to (&builder, top_x2, top_y);

        hisvg_path_builder_arc (&builder,
                               top_x2, top_y,
                               rx, ry, 0, FALSE, TRUE,
                               right_x, right_y1);

        hisvg_path_builder_line_to (&builder, right_x, right_y2);

        hisvg_path_builder_arc (&builder,
                               right_x, right_y2,
                               rx, ry, 0, FALSE, TRUE,
                               bottom_x2, bottom_y);

        hisvg_path_builder_line_to (&builder, bottom_x1, bottom_y);

        hisvg_path_builder_arc (&builder,
                               bottom_x1, bottom_y,
                               rx, ry, 0, FALSE, TRUE,
                               left_x, left_y2);

        hisvg_path_builder_line_to (&builder, left_x, left_y1);

        hisvg_path_builder_arc (&builder,
                               left_x, left_y1,
                               rx, ry, 0, FALSE, TRUE,
                               top_x1, top_y);

        hisvg_path_builder_close_path (&builder);
    }

    path = hisvg_path_builder_finish (&builder);

    hisvg_state_reinherit_top (ctx, self->state, dominate);
    hisvg_render_path (ctx, path);
    hisvg_cairo_path_destroy (path);
}

HiSVGNode *
hisvg_new_rect (const char* name)
{
    HiSVGNodeRect *rect;
    rect = g_new (HiSVGNodeRect, 1);
    _hisvg_node_init (&rect->super, HISVG_NODE_TYPE_RECT, name);
    rect->super.draw = _hisvg_node_rect_draw;
    rect->super.set_atts = _hisvg_node_rect_set_atts;
    rect->x = rect->y = rect->w = rect->h = rect->rx = rect->ry = _hisvg_css_parse_length ("0");
    rect->got_rx = rect->got_ry = FALSE;
    return &rect->super;
}

struct _HiSVGNodeCircle {
    HiSVGNode super;
    HiSVGLength cx, cy, r;
};

typedef struct _HiSVGNodeCircle HiSVGNodeCircle;

static void
_hisvg_node_circle_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeCircle *circle = (HiSVGNodeCircle *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "cx")))
            circle->cx = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "cy")))
            circle->cy = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "r")))
            circle->r = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "circle", klazz, id, atts);
    }
}

static void
_hisvg_node_circle_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    cairo_path_t *path;
    HiSVGNodeCircle *circle = (HiSVGNodeCircle *) self;
    double cx, cy, r;
    HiSVGPathBuilder builder;

    cx = _hisvg_css_normalize_length (&circle->cx, ctx, 'h');
    cy = _hisvg_css_normalize_length (&circle->cy, ctx, 'v');
    r = _hisvg_css_normalize_length (&circle->r, ctx, 'o');

    if (r <= 0)
        return;

    /* approximate a circle using 4 bezier curves */

    hisvg_path_builder_init (&builder, 19);

    hisvg_path_builder_move_to (&builder, cx + r, cy);

    hisvg_path_builder_curve_to (&builder,
                                cx + r, cy + r * HISVG_ARC_MAGIC,
                                cx + r * HISVG_ARC_MAGIC, cy + r,
                                cx, cy + r);

    hisvg_path_builder_curve_to (&builder,
                                cx - r * HISVG_ARC_MAGIC, cy + r,
                                cx - r, cy + r * HISVG_ARC_MAGIC,
                                cx - r, cy);

    hisvg_path_builder_curve_to (&builder,
                                cx - r, cy - r * HISVG_ARC_MAGIC,
                                cx - r * HISVG_ARC_MAGIC, cy - r,
                                cx, cy - r);

    hisvg_path_builder_curve_to (&builder,
                                cx + r * HISVG_ARC_MAGIC, cy - r,
                                cx + r, cy - r * HISVG_ARC_MAGIC,
                                cx + r, cy);

    hisvg_path_builder_close_path (&builder);

    path = hisvg_path_builder_finish (&builder);

    hisvg_state_reinherit_top (ctx, self->state, dominate);
    hisvg_render_path (ctx, path);
    hisvg_cairo_path_destroy (path);
}

HiSVGNode *
hisvg_new_circle (const char* name)
{
    HiSVGNodeCircle *circle;
    circle = g_new (HiSVGNodeCircle, 1);
    _hisvg_node_init (&circle->super, HISVG_NODE_TYPE_CIRCLE, name);
    circle->super.draw = _hisvg_node_circle_draw;
    circle->super.set_atts = _hisvg_node_circle_set_atts;
    circle->cx = circle->cy = circle->r = _hisvg_css_parse_length ("0");
    return &circle->super;
}

struct _HiSVGNodeEllipse {
    HiSVGNode super;
    HiSVGLength cx, cy, rx, ry;
};

typedef struct _HiSVGNodeEllipse HiSVGNodeEllipse;

static void
_hisvg_node_ellipse_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeEllipse *ellipse = (HiSVGNodeEllipse *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "cx")))
            ellipse->cx = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "cy")))
            ellipse->cy = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "rx")))
            ellipse->rx = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "ry")))
            ellipse->ry = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, value, self);
        }

        hisvg_parse_style_attrs (ctx, self->state, "ellipse", klazz, id, atts);
    }
}

static void
_hisvg_node_ellipse_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGNodeEllipse *ellipse = (HiSVGNodeEllipse *) self;
    cairo_path_t *path;
    double cx, cy, rx, ry;
    HiSVGPathBuilder builder;

    cx = _hisvg_css_normalize_length (&ellipse->cx, ctx, 'h');
    cy = _hisvg_css_normalize_length (&ellipse->cy, ctx, 'v');
    rx = _hisvg_css_normalize_length (&ellipse->rx, ctx, 'h');
    ry = _hisvg_css_normalize_length (&ellipse->ry, ctx, 'v');

    if (rx <= 0 || ry <= 0)
        return;

    /* approximate an ellipse using 4 bezier curves */

    hisvg_path_builder_init (&builder, 19);

    hisvg_path_builder_move_to (&builder, cx + rx, cy);

    hisvg_path_builder_curve_to (&builder,
                                cx + rx, cy - HISVG_ARC_MAGIC * ry,
                                cx + HISVG_ARC_MAGIC * rx, cy - ry,
                                cx, cy - ry);

    hisvg_path_builder_curve_to (&builder,
                                cx - HISVG_ARC_MAGIC * rx, cy - ry,
                                cx - rx, cy - HISVG_ARC_MAGIC * ry,
                                cx - rx, cy);

    hisvg_path_builder_curve_to (&builder,
                                cx - rx, cy + HISVG_ARC_MAGIC * ry,
                                cx - HISVG_ARC_MAGIC * rx, cy + ry,
                                cx, cy + ry);

    hisvg_path_builder_curve_to (&builder,
                                cx + HISVG_ARC_MAGIC * rx, cy + ry,
                                cx + rx, cy + HISVG_ARC_MAGIC * ry,
                                cx + rx, cy);

    hisvg_path_builder_close_path (&builder);

    path = hisvg_path_builder_finish (&builder);

    hisvg_state_reinherit_top (ctx, self->state, dominate);
    hisvg_render_path (ctx, path);
    hisvg_cairo_path_destroy (path);
}

HiSVGNode *
hisvg_new_ellipse (const char* name)
{
    HiSVGNodeEllipse *ellipse;
    ellipse = g_new (HiSVGNodeEllipse, 1);
    _hisvg_node_init (&ellipse->super, HISVG_NODE_TYPE_ELLIPSE, name);
    ellipse->super.draw = _hisvg_node_ellipse_draw;
    ellipse->super.set_atts = _hisvg_node_ellipse_set_atts;
    ellipse->cx = ellipse->cy = ellipse->rx = ellipse->ry = _hisvg_css_parse_length ("0");
    return &ellipse->super;
}
