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
    Caleb Moore <c.moore@student.unsw.edu.au>
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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <math.h>
#include <string.h>

#include "hisvg-common.h"
#include "hisvg-private.h"
#include "hisvg-cairo-draw.h"
#include "hisvg-cairo-render.h"
#include "hisvg-styles.h"
#include "hisvg-structure.h"

static void
hisvg_cairo_render_free (HiSVGRender * self)
{
    HiSVGCairoRender *me = HISVG_CAIRO_RENDER (self);

    /* TODO */

    g_free (me);
}

HiSVGCairoRender *
hisvg_cairo_render_new (cairo_t * cr, double width, double height)
{
    HiSVGCairoRender *cairo_render = g_new0 (HiSVGCairoRender, 1);

    cairo_render->super.type = HISVG_RENDER_TYPE_CAIRO;
    cairo_render->super.free = hisvg_cairo_render_free;
    cairo_render->super.create_text_context = hisvg_cairo_create_text_context;
    cairo_render->super.render_text = hisvg_cairo_render_text;
    cairo_render->super.render_surface = hisvg_cairo_render_surface;
    cairo_render->super.render_path = hisvg_cairo_render_path;
    cairo_render->super.pop_discrete_layer = hisvg_cairo_pop_discrete_layer;
    cairo_render->super.push_discrete_layer = hisvg_cairo_push_discrete_layer;
    cairo_render->super.add_clipping_rect = hisvg_cairo_add_clipping_rect;
    cairo_render->super.get_surface_of_node = hisvg_cairo_get_surface_of_node;
    cairo_render->width = width;
    cairo_render->height = height;
    cairo_render->offset_x = 0;
    cairo_render->offset_y = 0;
    cairo_render->initial_cr = cr;
    cairo_render->cr = cr;
    cairo_render->cr_stack = NULL;
    cairo_render->bb_stack = NULL;
    cairo_render->surfaces_stack = NULL;

    return cairo_render;
}

static void hisvg_cairo_transformed_image_bounding_box (
    cairo_matrix_t * transform,
    double width, double height,
    double *x0, double *y0, double *x1, double *y1)
{
    double x00 = 0, x01 = 0, x10 = width, x11 = width;
    double y00 = 0, y01 = height, y10 = 0, y11 = height;
    double t;

    /* transform the four corners of the image */
    cairo_matrix_transform_point (transform, &x00, &y00);
    cairo_matrix_transform_point (transform, &x01, &y01);
    cairo_matrix_transform_point (transform, &x10, &y10);
    cairo_matrix_transform_point (transform, &x11, &y11);

    /* find minimum and maximum coordinates */
    t = x00  < x01 ? x00  : x01;
    t = t < x10 ? t : x10;
    *x0 = floor (t < x11 ? t : x11);

    t = y00  < y01 ? y00  : y01;
    t = t < y10 ? t : y10;
    *y0 = floor (t < y11 ? t : y11);

    t = x00  > x01 ? x00  : x01;
    t = t > x10 ? t : x10;
    *x1 = ceil (t > x11 ? t : x11);

    t = y00  > y01 ? y00  : y01;
    t = t > y10 ? t : y10;
    *y1 = ceil (t > y11 ? t : y11);
}

HiSVGDrawingCtx *
hisvg_cairo_new_drawing_ctx (cairo_t * cr, HiSVGHandle * handle)
{
    return hisvg_cairo_new_drawing_ctx_with_viewport(cr, handle, NULL);
}

HiSVGDrawingCtx *hisvg_cairo_new_drawing_ctx_with_viewport	(cairo_t * cr,
        HiSVGHandle * handle, const HiSVGRect* viewport)
{
    HiSVGDimensionData data;
    HiSVGDrawingCtx *draw;
    HiSVGCairoRender *render;
    HiSVGState *state;
    cairo_matrix_t affine;
    double bbx0, bby0, bbx1, bby1;

    hisvg_handle_get_dimensions_x (handle, &data);
    if (data.width == 0 || data.height == 0)
        return NULL;

    draw = g_new (HiSVGDrawingCtx, 1);

    cairo_get_matrix (cr, &affine);

    if (viewport)
    {
        data.width = viewport->width;
        data.height = viewport->height;
        cairo_matrix_init_translate(&affine, viewport->x, viewport->y);
    }
    /* find bounding box of image as transformed by the current cairo context
     * The size of this bounding box determines the size of the intermediate
     * surfaces allocated during drawing. */
    hisvg_cairo_transformed_image_bounding_box (&affine,
                                               data.width, data.height,
                                               &bbx0, &bby0, &bbx1, &bby1);

    render = hisvg_cairo_render_new (cr, bbx1 - bbx0, bby1 - bby0);

    if (!render)
        return NULL;

    draw->render = (HiSVGRender *) render;
    render->offset_x = bbx0;
    render->offset_y = bby0;

    draw->state = NULL;

    draw->defs = handle->priv->defs;
    draw->dpi_x = handle->priv->dpi_x;
    draw->dpi_y = handle->priv->dpi_y;
    draw->vb.rect.width = data.em;
    draw->vb.rect.height = data.ex;
    draw->text_context = NULL;
    draw->drawsub_stack = NULL;
    draw->acquired_nodes = NULL;

    hisvg_state_push (draw);
    state = hisvg_current_state (draw);

    /* apply cairo transformation to our affine transform */
    cairo_matrix_multiply (&state->affine, &affine, &state->affine);

    /* scale according to size set by size_func callback */
    cairo_matrix_init_scale (&affine, data.width / data.em, data.height / data.ex);
    cairo_matrix_multiply (&state->affine, &affine, &state->affine);

    /* adjust transform so that the corner of the bounding box above is
     * at (0,0) - we compensate for this in _set_hisvg_affine() in
     * hisvg-cairo-render.c and a few other places */
    state->affine.x0 -= render->offset_x;
    state->affine.y0 -= render->offset_y;

    hisvg_bbox_init (&((HiSVGCairoRender *) draw->render)->bbox, &state->affine);

    return draw;
}

gboolean
hisvg_handle_render_cairo (HiSVGHandle* handle, cairo_t* cr,
        const HiSVGRect* viewport, const char* id, GError** error)
{
    HiSVGDrawingCtx *draw;
    HiSVGNode *drawsub = NULL;

    g_return_val_if_fail (handle != NULL, FALSE);

    if (!handle->priv->finished)
        return FALSE;

    if (id && *id)
        drawsub = hisvg_defs_lookup (handle->priv->defs, id);

    if (drawsub == NULL && id != NULL) {
        /* todo: there's no way to signal that @id doesn't exist */
        return FALSE;
    }

    draw = hisvg_cairo_new_drawing_ctx_with_viewport (cr, handle, viewport);
    if (!draw)
        return FALSE;

    while (drawsub != NULL) {
        draw->drawsub_stack = g_slist_prepend (draw->drawsub_stack, drawsub);
        drawsub = HISVG_NODE_PARENT(drawsub);
    }

    hisvg_state_push (draw);
    cairo_save (cr);

    hisvg_node_draw ((HiSVGNode *) handle->priv->treebase, draw, 0);

    cairo_restore (cr);
    hisvg_state_pop (draw);
    hisvg_drawing_ctx_free (draw);

    return TRUE;
}


