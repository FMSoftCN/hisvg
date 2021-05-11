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

    Copyright (C) 2005 Dom Lachowicz <cinamod@hotmail.com>
    Copyright (C) 2005 Caleb Moore <c.moore@student.unsw.edu.au>
    Copyright (C) 2005 Red Hat, Inc.
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

#include "hisvg-cairo-draw.h"
#include "hisvg-cairo-render.h"
#include "hisvg-cairo-clip.h"
#include "hisvg-styles.h"
#include "hisvg-path.h"
#include "hisvg-filter.h"
#include "hisvg-structure.h"
#include "hisvg-image.h"

#include <math.h>
#include <string.h>

#include "hisvg-text-helper.h"

static const cairo_user_data_key_t surface_pixel_data_key;

static void
_pattern_add_hisvg_color_stops (cairo_pattern_t * pattern,
                               HiSVGNode* n, guint32 current_color_rgb, guint8 opacity)
{
    gsize i;
    HiSVGGradientStop *stop;
    HiSVGNode *node;
    guint32 rgba;
    HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(n->base);
    while(child)
    {
        HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
        child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
        if (HISVG_NODE_TYPE (node) != HISVG_NODE_TYPE_STOP)
            continue;
        stop = (HiSVGGradientStop *) node;
        rgba = stop->rgba;
        cairo_pattern_add_color_stop_rgba (pattern, stop->offset,
                                           ((rgba >> 24) & 0xff) / 255.0,
                                           ((rgba >> 16) & 0xff) / 255.0,
                                           ((rgba >> 8) & 0xff) / 255.0,
                                           (((rgba >> 0) & 0xff) * opacity) / 255.0 / 255.0);
    }
}

static void
_set_source_hisvg_linear_gradient (HiSVGDrawingCtx * ctx,
                                  HiSVGLinearGradient * linear,
                                  guint32 current_color_rgb, guint8 opacity, HiSVGBbox bbox)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_t *cr = render->cr;
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;
    HiSVGLinearGradient statlinear;
    statlinear = *linear;
    linear = &statlinear;
    hisvg_linear_gradient_fix_fallback (ctx, linear);

    if (linear->has_current_color)
        current_color_rgb = linear->current_color;

    if (linear->obj_bbox)
        _hisvg_push_view_box (ctx, 1., 1.);
    pattern = cairo_pattern_create_linear (_hisvg_css_normalize_length (&linear->x1, ctx, 'h'),
                                           _hisvg_css_normalize_length (&linear->y1, ctx, 'v'),
                                           _hisvg_css_normalize_length (&linear->x2, ctx, 'h'),
                                           _hisvg_css_normalize_length (&linear->y2, ctx, 'v'));

    if (linear->obj_bbox)
        _hisvg_pop_view_box (ctx);

    matrix = linear->affine;
    if (linear->obj_bbox) {
        cairo_matrix_t bboxmatrix;
        cairo_matrix_init (&bboxmatrix, bbox.rect.width, 0, 0, bbox.rect.height,
                           bbox.rect.x, bbox.rect.y);
        cairo_matrix_multiply (&matrix, &matrix, &bboxmatrix);
    }
    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);
    cairo_pattern_set_extend (pattern, linear->spread);

    _pattern_add_hisvg_color_stops (pattern, (HiSVGNode*)&linear, current_color_rgb, opacity);

    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

static void
_set_source_hisvg_radial_gradient (HiSVGDrawingCtx * ctx,
                                  HiSVGRadialGradient * radial,
                                  guint32 current_color_rgb, guint8 opacity, HiSVGBbox bbox)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_t *cr = render->cr;
    cairo_pattern_t *pattern;
    cairo_matrix_t matrix;
    HiSVGRadialGradient statradial;
    statradial = *radial;
    radial = &statradial;
    hisvg_radial_gradient_fix_fallback (ctx, radial);

    if (radial->has_current_color)
        current_color_rgb = radial->current_color;

    if (radial->obj_bbox)
        _hisvg_push_view_box (ctx, 1., 1.);

    pattern = cairo_pattern_create_radial (_hisvg_css_normalize_length (&radial->fx, ctx, 'h'),
                                           _hisvg_css_normalize_length (&radial->fy, ctx, 'v'), 0.0,
                                           _hisvg_css_normalize_length (&radial->cx, ctx, 'h'),
                                           _hisvg_css_normalize_length (&radial->cy, ctx, 'v'),
                                           _hisvg_css_normalize_length (&radial->r, ctx, 'o'));
    if (radial->obj_bbox)
        _hisvg_pop_view_box (ctx);

    matrix = radial->affine;
    if (radial->obj_bbox) {
        cairo_matrix_t bboxmatrix;
        cairo_matrix_init (&bboxmatrix, bbox.rect.width, 0, 0, bbox.rect.height,
                           bbox.rect.x, bbox.rect.y);
        cairo_matrix_multiply (&matrix, &matrix, &bboxmatrix);
    }

    cairo_matrix_invert (&matrix);
    cairo_pattern_set_matrix (pattern, &matrix);
    cairo_pattern_set_extend (pattern, radial->spread);

    _pattern_add_hisvg_color_stops (pattern, (HiSVGNode*)&radial, current_color_rgb, opacity);

    cairo_set_source (cr, pattern);
    cairo_pattern_destroy (pattern);
}

static void
_set_source_hisvg_solid_color (HiSVGDrawingCtx * ctx,
                              HiSVGSolidColor * color, guint8 opacity, guint32 current_color)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_t *cr = render->cr;
    guint32 argb = color->argb;
    double r, g, b, a;

    if (color->currentcolor)
        argb = current_color;

    r = ((argb >> 16) & 0xff) / 255.0;
    g = ((argb >>  8) & 0xff) / 255.0;
    b = ((argb >>  0) & 0xff) / 255.0;
    a =  (argb >> 24) / 255.0 * (opacity / 255.0);

    cairo_set_source_rgba (cr, r, g, b, a);
}

static void
_set_source_hisvg_pattern (HiSVGDrawingCtx * ctx,
                          HiSVGPattern * hisvg_pattern, guint8 opacity, HiSVGBbox bbox)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    HiSVGPattern local_pattern = *hisvg_pattern;
    cairo_t *cr_render, *cr_pattern;
    cairo_pattern_t *pattern;
    cairo_surface_t *surface;
    cairo_matrix_t matrix;
    cairo_matrix_t affine, caffine, taffine;
    double bbwscale, bbhscale, scwscale, schscale;
    double patternw, patternh, patternx, patterny;
    int pw, ph;

    hisvg_pattern = &local_pattern;
    hisvg_pattern_fix_fallback (ctx, hisvg_pattern);
    cr_render = render->cr;

    if (hisvg_pattern->obj_bbox)
        _hisvg_push_view_box (ctx, 1., 1.);

    patternx = _hisvg_css_normalize_length (&hisvg_pattern->x, ctx, 'h');
    patterny = _hisvg_css_normalize_length (&hisvg_pattern->y, ctx, 'v');
    patternw = _hisvg_css_normalize_length (&hisvg_pattern->width, ctx, 'h');
    patternh = _hisvg_css_normalize_length (&hisvg_pattern->height, ctx, 'v');

    if (hisvg_pattern->obj_bbox)
        _hisvg_pop_view_box (ctx);


    /* Work out the size of the rectangle so it takes into account the object bounding box */


    if (hisvg_pattern->obj_bbox) {
        bbwscale = bbox.rect.width;
        bbhscale = bbox.rect.height;
    } else {
        bbwscale = 1.0;
        bbhscale = 1.0;
    }

    cairo_matrix_multiply (&taffine, &hisvg_pattern->affine, &hisvg_current_state (ctx)->affine);

    scwscale = sqrt (taffine.xx * taffine.xx + taffine.xy * taffine.xy);
    schscale = sqrt (taffine.yx * taffine.yx + taffine.yy * taffine.yy);

    pw = patternw * bbwscale * scwscale;
    ph = patternh * bbhscale * schscale;

    scwscale = (double) pw / (double) (patternw * bbwscale);
    schscale = (double) ph / (double) (patternh * bbhscale);

    surface = cairo_surface_create_similar (cairo_get_target (cr_render),
                                            CAIRO_CONTENT_COLOR_ALPHA, pw, ph);
    cr_pattern = cairo_create (surface);

    /* Create the pattern coordinate system */
    if (hisvg_pattern->obj_bbox) {
        /* subtract the pattern origin */
        cairo_matrix_init_translate (&affine,
                                     bbox.rect.x + patternx * bbox.rect.width,
                                     bbox.rect.y + patterny * bbox.rect.height);
    } else {
        /* subtract the pattern origin */
        cairo_matrix_init_translate (&affine, patternx, patterny);
    }
    /* Apply the pattern transform */
    cairo_matrix_multiply (&affine, &affine, &hisvg_pattern->affine);

    /* Create the pattern contents coordinate system */
    if (hisvg_pattern->vbox.active) {
        /* If there is a vbox, use that */
        double w, h, x, y;
        w = patternw * bbwscale;
        h = patternh * bbhscale;
        x = 0;
        y = 0;
        hisvg_preserve_aspect_ratio (hisvg_pattern->preserve_aspect_ratio,
                                    hisvg_pattern->vbox.rect.width, hisvg_pattern->vbox.rect.height,
                                    &w, &h, &x, &y);

        x -= hisvg_pattern->vbox.rect.x * w / hisvg_pattern->vbox.rect.width;
        y -= hisvg_pattern->vbox.rect.y * h / hisvg_pattern->vbox.rect.height;

        cairo_matrix_init (&caffine,
                           w / hisvg_pattern->vbox.rect.width,
                           0,
                           0,
                           h / hisvg_pattern->vbox.rect.height,
                           x,
                           y);
        _hisvg_push_view_box (ctx, hisvg_pattern->vbox.rect.width, hisvg_pattern->vbox.rect.height);
    } else if (hisvg_pattern->obj_cbbox) {
        /* If coords are in terms of the bounding box, use them */
        cairo_matrix_init_scale (&caffine, bbox.rect.width, bbox.rect.height);
        _hisvg_push_view_box (ctx, 1., 1.);
    } else {
        cairo_matrix_init_identity (&caffine);
    }

    if (scwscale != 1.0 || schscale != 1.0) {
        cairo_matrix_t scalematrix;

        cairo_matrix_init_scale (&scalematrix, scwscale, schscale);
        cairo_matrix_multiply (&caffine, &caffine, &scalematrix);
        cairo_matrix_init_scale (&scalematrix, 1. / scwscale, 1. / schscale);
        cairo_matrix_multiply (&affine, &scalematrix, &affine);
    }

    /* Draw to another surface */
    render->cr = cr_pattern;

    /* Set up transformations to be determined by the contents units */
    hisvg_state_push (ctx);
    hisvg_current_state (ctx)->personal_affine =
            hisvg_current_state (ctx)->affine = caffine;

    /* Draw everything */
    _hisvg_node_draw_children ((HiSVGNode *) hisvg_pattern, ctx, 2);
    /* Return to the original coordinate system */
    hisvg_state_pop (ctx);

    /* Set the render to draw where it used to */
    render->cr = cr_render;

    pattern = cairo_pattern_create_for_surface (surface);
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

    matrix = affine;
    if (cairo_matrix_invert (&matrix) != CAIRO_STATUS_SUCCESS)
      goto out;

    cairo_pattern_set_matrix (pattern, &matrix);
    cairo_pattern_set_filter (pattern, CAIRO_FILTER_BEST);

    cairo_set_source (cr_render, pattern);

    cairo_pattern_destroy (pattern);
    cairo_destroy (cr_pattern);
    cairo_surface_destroy (surface);

  out:
    if (hisvg_pattern->obj_cbbox || hisvg_pattern->vbox.active)
        _hisvg_pop_view_box (ctx);
}

/* note: _set_source_hisvg_paint_server does not change cairo's CTM */
static void
_set_source_hisvg_paint_server (HiSVGDrawingCtx * ctx,
                               guint32 current_color_rgb,
                               HiSVGPaintServer * ps,
                               guint8 opacity, HiSVGBbox bbox, guint32 current_color)
{
    HiSVGNode *node;

    switch (ps->type) {
    case HISVG_PAINT_SERVER_IRI:
        node = hisvg_acquire_node (ctx, ps->core.iri);
        if (node == NULL)
            break;
        else if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_LINEAR_GRADIENT)
            _set_source_hisvg_linear_gradient (ctx, (HiSVGLinearGradient *) node, current_color_rgb, opacity, bbox);
        else if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_RADIAL_GRADIENT)
            _set_source_hisvg_radial_gradient (ctx, (HiSVGRadialGradient *) node, current_color_rgb, opacity, bbox);
        else if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_PATTERN)
            _set_source_hisvg_pattern (ctx, (HiSVGPattern *) node, opacity, bbox);
        hisvg_release_node (ctx, node);
        break;
    case HISVG_PAINT_SERVER_SOLID:
        _set_source_hisvg_solid_color (ctx, ps->core.color, opacity, current_color);
        break;
    }
}

static void
_set_hisvg_affine (HiSVGCairoRender * render, cairo_matrix_t *affine)
{
    cairo_t * cr = render->cr;
    cairo_matrix_t matrix;
    gboolean nest = cr != render->initial_cr;

    cairo_matrix_init (&matrix,
                       affine->xx, affine->yx,
                       affine->xy, affine->yy,
                       affine->x0 + (nest ? 0 : render->offset_x),
                       affine->y0 + (nest ? 0 : render->offset_y));
    cairo_set_matrix (cr, &matrix);
}

void *
hisvg_cairo_create_text_context (HiSVGDrawingCtx * ctx, HiSVGState * state)
{
    HiSVGTextContext *context;
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);

    HiSVGTextDirection* direction =  NULL;
    if (state->unicode_bidi == UNICODE_BIDI_OVERRIDE || state->unicode_bidi == UNICODE_BIDI_EMBED)
    {
        direction = &state->text_dir;
    }

    HiSVGTextGravity* gravity = NULL;
    if (HISVG_TEXT_GRAVITY_IS_VERTICAL (state->text_gravity))
    {
        gravity = &state->text_gravity;
    }
    context = hisvg_text_context_create (ctx->dpi_y, 
            state->lang, direction, gravity);

    hisvg_cairo_update_text_context (render->cr, context);
    return context;
}

void
hisvg_cairo_render_text (HiSVGDrawingCtx * ctx, void* lyt, double x, double y)
{
    HiSVGTextContextLayout* layout = lyt;
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    HiSVGState *state = hisvg_current_state (ctx);
    HiSVGTextRectangle rect;
    HiSVGBbox bbox;
    HiSVGTextGravity gravity = hisvg_text_context_get_gravity (hisvg_text_layout_get_context (layout));
    double rotation;

    cairo_set_antialias (render->cr, state->text_rendering_type);

    _set_hisvg_affine (render, &state->affine);

    hisvg_text_context_layout_get_rect (layout, &rect);

    hisvg_bbox_init (&bbox, &state->affine);
    if (HISVG_TEXT_GRAVITY_IS_VERTICAL (gravity)) {
        bbox.rect.x = x + (rect.x - rect.height) / (double)HISVG_TEXT_SCALE;
        bbox.rect.y = y + rect.y / (double)HISVG_TEXT_SCALE;
        bbox.rect.width = rect.height / (double)HISVG_TEXT_SCALE;
        bbox.rect.height = rect.width / (double)HISVG_TEXT_SCALE;
    } else {
        bbox.rect.x = x + rect.x / (double)HISVG_TEXT_SCALE;
        bbox.rect.y = y + rect.y / (double)HISVG_TEXT_SCALE;
        bbox.rect.width = rect.width / (double)HISVG_TEXT_SCALE;
        bbox.rect.height = rect.height / (double)HISVG_TEXT_SCALE;
    }
    bbox.virgin = 0;

    rotation = hisvg_text_gravity_to_rotation (gravity);
    if (state->fill) {
        cairo_save (render->cr);
        cairo_move_to (render->cr, x, y);
        hisvg_bbox_insert (&render->bbox, &bbox);
        _set_source_hisvg_paint_server (ctx,
                                       state->current_color,
                                       state->fill,
                                       state->fill_opacity,
                                       bbox, hisvg_current_state (ctx)->current_color);
        if (rotation != 0.)
            cairo_rotate (render->cr, -rotation);
        hisvg_cairo_show_layout (render->cr, layout);
        cairo_restore (render->cr);
    }

    if (state->stroke) {
        cairo_save (render->cr);
        cairo_move_to (render->cr, x, y);
        hisvg_bbox_insert (&render->bbox, &bbox);

        _set_source_hisvg_paint_server (ctx,
                                       state->current_color,
                                       state->stroke,
                                       state->stroke_opacity,
                                       bbox, hisvg_current_state (ctx)->current_color);

        if (rotation != 0.)
            cairo_rotate (render->cr, -rotation);
        hisvg_cairo_layout_path (render->cr, layout);

        cairo_set_line_width (render->cr, _hisvg_css_normalize_length (&state->stroke_width, ctx, 'h'));
        cairo_set_miter_limit (render->cr, state->miter_limit);
        cairo_set_line_cap (render->cr, (cairo_line_cap_t) state->cap);
        cairo_set_line_join (render->cr, (cairo_line_join_t) state->join);
        cairo_set_dash (render->cr, state->dash.dash, state->dash.n_dash,
                        _hisvg_css_normalize_length (&state->dash.offset, ctx, 'o'));
        cairo_stroke (render->cr);
        cairo_restore (render->cr);
    }
}

void
hisvg_cairo_render_path (HiSVGDrawingCtx * ctx, const cairo_path_t *path)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    HiSVGState *state = hisvg_current_state (ctx);
    cairo_t *cr;
    HiSVGBbox bbox;
    double backup_tolerance;

    hisvg_cairo_push_discrete_layer (ctx);

    cr = render->cr;

    cairo_set_antialias (cr, state->shape_rendering_type);

    _set_hisvg_affine (render, &state->affine);

    cairo_set_line_width (cr, _hisvg_css_normalize_length (&state->stroke_width, ctx, 'h'));
    cairo_set_miter_limit (cr, state->miter_limit);
    cairo_set_line_cap (cr, (cairo_line_cap_t) state->cap);
    cairo_set_line_join (cr, (cairo_line_join_t) state->join);
    cairo_set_dash (cr, state->dash.dash, state->dash.n_dash,
                    _hisvg_css_normalize_length (&state->dash.offset, ctx, 'o'));

    cairo_append_path (cr, path);

    hisvg_bbox_init (&bbox, &state->affine);

    backup_tolerance = cairo_get_tolerance (cr);
    cairo_set_tolerance (cr, 1.0);
    /* dropping the precision of cairo's bezier subdivision, yielding 2x
       _rendering_ time speedups, are these rather expensive operations
       really needed here? */

    /* Bounding box for fill
     *
     * Unlike the case for stroke, for fills we always compute the bounding box.
     * In GNOME we have SVGs for symbolic icons where each icon has a bounding
     * rectangle with no fill and no stroke, and inside it there are the actual
     * paths for the icon's shape.  We need to be able to compute the bounding
     * rectangle's extents, even when it has no fill nor stroke.
     */
    {
        HiSVGBbox fb;
        hisvg_bbox_init (&fb, &state->affine);
        cairo_fill_extents (cr, &fb.rect.x, &fb.rect.y, &fb.rect.width, &fb.rect.height);
        fb.rect.width -= fb.rect.x;
        fb.rect.height -= fb.rect.y;
        fb.virgin = 0;
        hisvg_bbox_insert (&bbox, &fb);
    }

    /* Bounding box for stroke */
    if (state->stroke != NULL) {
        HiSVGBbox sb;
        hisvg_bbox_init (&sb, &state->affine);
        cairo_stroke_extents (cr, &sb.rect.x, &sb.rect.y, &sb.rect.width, &sb.rect.height);
        sb.rect.width -= sb.rect.x;
        sb.rect.height -= sb.rect.y;
        sb.virgin = 0;
        hisvg_bbox_insert (&bbox, &sb);
    }

    cairo_set_tolerance (cr, backup_tolerance);

    hisvg_bbox_insert (&render->bbox, &bbox);

    if (state->fill != NULL) {
        int opacity;

        cairo_set_fill_rule (cr, state->fill_rule);

        opacity = state->fill_opacity;

        _set_source_hisvg_paint_server (ctx,
                                       state->current_color,
                                       state->fill,
                                       opacity, bbox, hisvg_current_state (ctx)->current_color);

        if (state->stroke != NULL)
            cairo_fill_preserve (cr);
        else
            cairo_fill (cr);
    }

    if (state->stroke != NULL) {
        int opacity;
        opacity = state->stroke_opacity;

        _set_source_hisvg_paint_server (ctx,
                                       state->current_color,
                                       state->stroke,
                                       opacity, bbox, hisvg_current_state (ctx)->current_color);

        cairo_stroke (cr);
    }

    cairo_new_path (cr); /* clear the path in case stroke == fill == NULL; otherwise we leave it around from computing the bounding box */

    hisvg_cairo_pop_discrete_layer (ctx);
}

void
hisvg_cairo_render_surface (HiSVGDrawingCtx *ctx, 
                           cairo_surface_t *surface,
                           double src_x, 
                           double src_y, 
                           double w, 
                           double h)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    HiSVGState *state = hisvg_current_state (ctx);

    int width, height;
    double dwidth, dheight;
    HiSVGBbox bbox;

    if (surface == NULL)
        return;

    g_return_if_fail (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE);

    dwidth = width = cairo_image_surface_get_width (surface);
    dheight = height = cairo_image_surface_get_height (surface);
    if (width == 0 || height == 0)
        return;

    hisvg_bbox_init (&bbox, &state->affine);
    bbox.rect.x = src_x;
    bbox.rect.y = src_y;
    bbox.rect.width = w;
    bbox.rect.height = h;
    bbox.virgin = 0;

    _set_hisvg_affine (render, &state->affine);
    cairo_scale (render->cr, w / dwidth, h / dheight);
    src_x *= dwidth / w;
    src_y *= dheight / h;

    cairo_set_operator (render->cr, state->comp_op);

#if 1
    cairo_set_source_surface (render->cr, surface, src_x, src_y);
#else
    {
        cairo_pattern_t *pattern;
        cairo_matrix_t matrix;

        pattern = cairo_pattern_create_for_surface (surface);
        cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);

        cairo_matrix_init_translate (&matrix, -src_x, -src_y);
        cairo_pattern_set_matrix (pattern, &matrix);

        cairo_set_source (render->cr, pattern);
        cairo_pattern_destroy (pattern);
    }
#endif

    cairo_paint (render->cr);

    hisvg_bbox_insert (&render->bbox, &bbox);
}

static void
hisvg_cairo_generate_mask (cairo_t * cr, HiSVGMask * self, HiSVGDrawingCtx * ctx, HiSVGBbox * bbox)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_surface_t *surface;
    cairo_t *mask_cr, *save_cr;
    HiSVGState *state = hisvg_current_state (ctx);
    guint8 *pixels;
    guint32 width = render->width, height = render->height;
    guint32 rowstride = width * 4, row, i;
    cairo_matrix_t affinesave;
    double sx, sy, sw, sh;
    gboolean nest = cr != render->initial_cr;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy (surface);
        return;
    }

    pixels = cairo_image_surface_get_data (surface);
    rowstride = cairo_image_surface_get_stride (surface);

    if (self->maskunits == objectBoundingBox)
        _hisvg_push_view_box (ctx, 1, 1);

    sx = _hisvg_css_normalize_length (&self->x, ctx, 'h');
    sy = _hisvg_css_normalize_length (&self->y, ctx, 'v');
    sw = _hisvg_css_normalize_length (&self->width, ctx, 'h');
    sh = _hisvg_css_normalize_length (&self->height, ctx, 'v');

    if (self->maskunits == objectBoundingBox)
        _hisvg_pop_view_box (ctx);

    mask_cr = cairo_create (surface);
    save_cr = render->cr;
    render->cr = mask_cr;

    if (self->maskunits == objectBoundingBox)
        hisvg_cairo_add_clipping_rect (ctx,
                                      sx * bbox->rect.width + bbox->rect.x,
                                      sy * bbox->rect.height + bbox->rect.y,
                                      sw * bbox->rect.width,
                                      sh * bbox->rect.height);
    else
        hisvg_cairo_add_clipping_rect (ctx, sx, sy, sw, sh);

    /* Horribly dirty hack to have the bbox premultiplied to everything */
    if (self->contentunits == objectBoundingBox) {
        cairo_matrix_t bbtransform;
        cairo_matrix_init (&bbtransform,
                           bbox->rect.width,
                           0,
                           0,
                           bbox->rect.height,
                           bbox->rect.x,
                           bbox->rect.y);
        affinesave = self->super.state->affine;
        cairo_matrix_multiply (&self->super.state->affine, &bbtransform, &self->super.state->affine);
        _hisvg_push_view_box (ctx, 1, 1);
    }

    hisvg_state_push (ctx);
    _hisvg_node_draw_children (&self->super, ctx, 0);
    hisvg_state_pop (ctx);

    if (self->contentunits == objectBoundingBox) {
        _hisvg_pop_view_box (ctx);
        self->super.state->affine = affinesave;
    }

    render->cr = save_cr;

    for (row = 0; row < height; row++) {
        guint8 *row_data = (pixels + (row * rowstride));
        for (i = 0; i < width; i++) {
            guint32 *pixel = (guint32 *) row_data + i;
            *pixel = ((((*pixel & 0x00ff0000) >> 16) * 13817 +
                       ((*pixel & 0x0000ff00) >> 8) * 46518 +
                       ((*pixel & 0x000000ff)) * 4688) * state->opacity);
        }
    }

    cairo_destroy (mask_cr);

    cairo_identity_matrix (cr);
    cairo_mask_surface (cr, surface,
                        nest ? 0 : render->offset_x,
                        nest ? 0 : render->offset_y);
    cairo_surface_destroy (surface);
}

static void
hisvg_cairo_push_render_stack (HiSVGDrawingCtx * ctx)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_surface_t *surface;
    cairo_t *child_cr;
    HiSVGBbox *bbox;
    HiSVGState *state = hisvg_current_state (ctx);
    gboolean lateclip = FALSE;

    if (hisvg_current_state (ctx)->clip_path) {
        HiSVGNode *node;
        node = hisvg_acquire_node (ctx, hisvg_current_state (ctx)->clip_path);
        if (node && HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_CLIP_PATH) {
            HiSVGClipPath *clip_path = (HiSVGClipPath *) node;

            switch (clip_path->units) {
            case userSpaceOnUse:
                hisvg_cairo_clip (ctx, clip_path, NULL);
                break;
            case objectBoundingBox:
                lateclip = TRUE;
                break;

            default:
                g_assert_not_reached ();
                break;
            }

        }
        
        hisvg_release_node (ctx, node);
    }

    if (state->opacity == 0xFF
        && !state->filter && !state->mask && !lateclip && (state->comp_op == CAIRO_OPERATOR_OVER)
        && (state->enable_background == HISVG_ENABLE_BACKGROUND_ACCUMULATE))
        return;

    if (!state->filter) {
        surface = cairo_surface_create_similar (cairo_get_target (render->cr),
                                                CAIRO_CONTENT_COLOR_ALPHA,
                                                render->width, render->height);
    } else {
        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 
                                              render->width, render->height);

        /* The surface reference is owned by the child_cr created below and put on the cr_stack! */
        render->surfaces_stack = g_list_prepend (render->surfaces_stack, surface);
    }

#if 0
    if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy (surface);
        return;
    }
#endif

    child_cr = cairo_create (surface);
    cairo_surface_destroy (surface);

    render->cr_stack = g_list_prepend (render->cr_stack, render->cr);
    render->cr = child_cr;

    bbox = g_new (HiSVGBbox, 1);
    *bbox = render->bbox;
    render->bb_stack = g_list_prepend (render->bb_stack, bbox);
    hisvg_bbox_init (&render->bbox, &state->affine);
}

void
hisvg_cairo_push_discrete_layer (HiSVGDrawingCtx * ctx)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);

    cairo_save (render->cr);
    hisvg_cairo_push_render_stack (ctx);
}

static void
hisvg_cairo_pop_render_stack (HiSVGDrawingCtx * ctx)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_t *child_cr = render->cr;
    HiSVGClipPath *lateclip = NULL;
    cairo_surface_t *surface = NULL;
    HiSVGState *state = hisvg_current_state (ctx);
    gboolean nest, needs_destroy = FALSE;

    if (hisvg_current_state (ctx)->clip_path) {
        HiSVGNode *node;
        node = hisvg_acquire_node (ctx, hisvg_current_state (ctx)->clip_path);
        if (node && HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_CLIP_PATH
            && ((HiSVGClipPath *) node)->units == objectBoundingBox)
            lateclip = (HiSVGClipPath *) node;
        else
            hisvg_release_node (ctx, node);
    }

    if (state->opacity == 0xFF
        && !state->filter && !state->mask && !lateclip && (state->comp_op == CAIRO_OPERATOR_OVER)
        && (state->enable_background == HISVG_ENABLE_BACKGROUND_ACCUMULATE))
        return;

    surface = cairo_get_target (child_cr);

    if (state->filter) {
        HiSVGNode *filter;
        cairo_surface_t *output;

        filter = hisvg_acquire_node (ctx, state->filter);
        output = render->surfaces_stack->data;
        render->surfaces_stack = g_list_delete_link (render->surfaces_stack, render->surfaces_stack);

        if (filter && HISVG_NODE_TYPE (filter) == HISVG_NODE_TYPE_FILTER) {
            needs_destroy = TRUE;
            surface = hisvg_filter_render ((HiSVGFilter *) filter, output, ctx, &render->bbox, "2103");
            /* Don't destroy the output surface, it's owned by child_cr */
        }

        hisvg_release_node (ctx, filter);
    }

    render->cr = (cairo_t *) render->cr_stack->data;
    render->cr_stack = g_list_delete_link (render->cr_stack, render->cr_stack);

    nest = render->cr != render->initial_cr;
    cairo_identity_matrix (render->cr);
    cairo_set_source_surface (render->cr, surface,
                              nest ? 0 : render->offset_x,
                              nest ? 0 : render->offset_y);

    if (lateclip) {
        hisvg_cairo_clip (ctx, lateclip, &render->bbox);
        hisvg_release_node (ctx, (HiSVGNode *) lateclip);
    }

    cairo_set_operator (render->cr, state->comp_op);

    if (state->mask) {
        HiSVGNode *mask;

        mask = hisvg_acquire_node (ctx, state->mask);
        if (mask && HISVG_NODE_TYPE (mask) == HISVG_NODE_TYPE_MASK)
          hisvg_cairo_generate_mask (render->cr, (HiSVGMask *) mask, ctx, &render->bbox);
        hisvg_release_node (ctx, mask);
    } else if (state->opacity != 0xFF)
        cairo_paint_with_alpha (render->cr, (double) state->opacity / 255.0);
    else
        cairo_paint (render->cr);

    cairo_destroy (child_cr);

    hisvg_bbox_insert ((HiSVGBbox *) render->bb_stack->data, &render->bbox);

    render->bbox = *((HiSVGBbox *) render->bb_stack->data);

    g_free (render->bb_stack->data);
    render->bb_stack = g_list_delete_link (render->bb_stack, render->bb_stack);

    if (needs_destroy) {
        cairo_surface_destroy (surface);
    }
}

void
hisvg_cairo_pop_discrete_layer (HiSVGDrawingCtx * ctx)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);

    hisvg_cairo_pop_render_stack (ctx);
    cairo_restore (render->cr);
}

void
hisvg_cairo_add_clipping_rect (HiSVGDrawingCtx * ctx, double x, double y, double w, double h)
{
    HiSVGCairoRender *render = HISVG_CAIRO_RENDER (ctx->render);
    cairo_t *cr = render->cr;

    _set_hisvg_affine (render, &hisvg_current_state (ctx)->affine);

    cairo_rectangle (cr, x, y, w, h);
    cairo_clip (cr);
}

cairo_surface_t *
hisvg_cairo_get_surface_of_node (HiSVGDrawingCtx *ctx,
                                HiSVGNode *drawable, 
                                double width, 
                                double height)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    HiSVGCairoRender *save_render = (HiSVGCairoRender *) ctx->render;
    HiSVGCairoRender *render;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy (surface);
        return NULL;
    }

    cr = cairo_create (surface);

    render = hisvg_cairo_render_new (cr, width, height);
    ctx->render = (HiSVGRender *) render;

    hisvg_state_push (ctx);
    hisvg_node_draw (drawable, ctx, 0);
    hisvg_state_pop (ctx);

    cairo_destroy (cr);

    hisvg_render_free (ctx->render);
    ctx->render = (HiSVGRender *) save_render;

    return surface;
}


/* Copied from gtk+/gdk/gdkpixbuf-drawable.c, LGPL 2+.
 *
 * Copyright (C) 1999 Michael Zucchi
 *
 * Authors: Michael Zucchi <zucchi@zedzone.mmc.com.au>
 *          Cody Russell <bratsche@dfw.net>
 *          Federico Mena-Quintero <federico@gimp.org>
 */

static void
convert_alpha (guchar *dest_data,
               int     dest_stride,
               guchar *src_data,
               int     src_stride,
               int     src_x,
               int     src_y,
               int     width,
               int     height)
{
    int x, y;

    src_data += src_stride * src_y + src_x * 4;

    for (y = 0; y < height; y++) {
        guint32 *src = (guint32 *) src_data;

        for (x = 0; x < width; x++) {
          guint alpha = src[x] >> 24;

          if (alpha == 0) {
              dest_data[x * 4 + 0] = 0;
              dest_data[x * 4 + 1] = 0;
              dest_data[x * 4 + 2] = 0;
          } else {
              dest_data[x * 4 + 0] = (((src[x] & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
              dest_data[x * 4 + 1] = (((src[x] & 0x00ff00) >>  8) * 255 + alpha / 2) / alpha;
              dest_data[x * 4 + 2] = (((src[x] & 0x0000ff) >>  0) * 255 + alpha / 2) / alpha;
          }
          dest_data[x * 4 + 3] = alpha;
      }

      src_data += src_stride;
      dest_data += dest_stride;
    }
}

static void
convert_no_alpha (guchar *dest_data,
                  int     dest_stride,
                  guchar *src_data,
                  int     src_stride,
                  int     src_x,
                  int     src_y,
                  int     width,
                  int     height)
{
    int x, y;

    src_data += src_stride * src_y + src_x * 4;

    for (y = 0; y < height; y++) {
        guint32 *src = (guint32 *) src_data;

        for (x = 0; x < width; x++) {
            dest_data[x * 3 + 0] = src[x] >> 16;
            dest_data[x * 3 + 1] = src[x] >>  8;
            dest_data[x * 3 + 2] = src[x];
        }

        src_data += src_stride;
        dest_data += dest_stride;
    }
}

