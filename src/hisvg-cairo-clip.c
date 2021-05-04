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

#include "hisvg-cairo-draw.h"
#include "hisvg-cairo-clip.h"
#include "hisvg-cairo-render.h"
#include "hisvg-styles.h"
#include "hisvg-path.h"

#include <math.h>
#include <string.h>

typedef struct HiSVGCairoClipRender HiSVGCairoClipRender;

struct HiSVGCairoClipRender {
    HiSVGCairoRender super;
    HiSVGCairoRender *parent;
};

#define HISVG_CAIRO_CLIP_RENDER(render) (_HISVG_RENDER_CIC ((render), HISVG_RENDER_TYPE_CAIRO_CLIP, HiSVGCairoClipRender))

static void
hisvg_cairo_clip_apply_affine (HiSVGCairoClipRender *render, cairo_matrix_t *affine)
{
    HiSVGCairoRender *cairo_render = &render->super;
    cairo_matrix_t matrix;
    gboolean nest = cairo_render->cr != cairo_render->initial_cr;

    cairo_matrix_init (&matrix,
                       affine->xx, affine->yx,
                       affine->xy, affine->yy,
                       affine->x0 + (nest ? 0 : render->parent->offset_x),
                       affine->y0 + (nest ? 0 : render->parent->offset_y));
    cairo_set_matrix (cairo_render->cr, &matrix);
}

static void
hisvg_cairo_clip_render_path (HiSVGDrawingCtx * ctx, const cairo_path_t *path)
{
    HiSVGCairoClipRender *render = HISVG_CAIRO_CLIP_RENDER (ctx->render);
    HiSVGCairoRender *cairo_render = &render->super;
    HiSVGState *state = hisvg_current_state (ctx);
    cairo_t *cr;

    cr = cairo_render->cr;

    hisvg_cairo_clip_apply_affine (render, &state->affine);

    cairo_set_fill_rule (cr, hisvg_current_state (ctx)->clip_rule);

    cairo_append_path (cr, path);
}

static void
hisvg_cairo_clip_render_surface (HiSVGDrawingCtx *ctx,
                                cairo_surface_t *surface,
                                double src_x,
                                double src_y, 
                                double w, 
                                double h)
{
}


static void
hisvg_cairo_clip_render_free (HiSVGRender * self)
{
    HiSVGCairoClipRender *clip_render = HISVG_CAIRO_CLIP_RENDER (self);

    g_free (clip_render);
}

static void
hisvg_cairo_clip_push_discrete_layer (HiSVGDrawingCtx * ctx)
{
}

static void
hisvg_cairo_clip_pop_discrete_layer (HiSVGDrawingCtx * ctx)
{
}

static void
hisvg_cairo_clip_add_clipping_rect (HiSVGDrawingCtx * ctx, double x, double y, double w, double h)
{
}

static HiSVGRender *
hisvg_cairo_clip_render_new (cairo_t * cr, HiSVGCairoRender *parent)
{
    HiSVGCairoClipRender *clip_render = g_new0 (HiSVGCairoClipRender, 1);
    HiSVGCairoRender *cairo_render = &clip_render->super;
    HiSVGRender *render = &cairo_render->super;

    g_assert (parent->super.type == HISVG_RENDER_TYPE_CAIRO);

    render->type = HISVG_RENDER_TYPE_CAIRO_CLIP;
    render->free = hisvg_cairo_clip_render_free;
    render->create_pango_context = hisvg_cairo_create_pango_context;
    render->render_pango_layout = hisvg_cairo_render_pango_layout;
    render->render_surface = hisvg_cairo_clip_render_surface;
    render->render_path = hisvg_cairo_clip_render_path;
    render->pop_discrete_layer = hisvg_cairo_clip_pop_discrete_layer;
    render->push_discrete_layer = hisvg_cairo_clip_push_discrete_layer;
    render->add_clipping_rect = hisvg_cairo_clip_add_clipping_rect;
    render->get_surface_of_node = NULL;
    cairo_render->initial_cr = parent->cr;
    cairo_render->cr = cr;
    clip_render->parent = parent;

    return render;
}

void
hisvg_cairo_clip (HiSVGDrawingCtx * ctx, HiSVGClipPath * clip, HiSVGBbox * bbox)
{
    HiSVGCairoRender *save = HISVG_CAIRO_RENDER (ctx->render);
    cairo_matrix_t affinesave;

    ctx->render = hisvg_cairo_clip_render_new (save->cr, save);

    /* Horribly dirty hack to have the bbox premultiplied to everything */
    if (clip->units == objectBoundingBox) {
        cairo_matrix_t bbtransform;
        cairo_matrix_init (&bbtransform,
                           bbox->rect.width,
                           0,
                           0,
                           bbox->rect.height,
                           bbox->rect.x,
                           bbox->rect.y);
        affinesave = clip->super.state->affine;
        cairo_matrix_multiply (&clip->super.state->affine, &bbtransform, &clip->super.state->affine);
    }

    hisvg_state_push (ctx);
    _hisvg_node_draw_children ((HiSVGNode *) clip, ctx, 0);
    hisvg_state_pop (ctx);

    if (clip->units == objectBoundingBox)
        clip->super.state->affine = affinesave;

    g_free (ctx->render);
    cairo_clip (save->cr);
    ctx->render = &save->super;
}
