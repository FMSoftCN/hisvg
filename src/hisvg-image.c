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


#include "hisvg-image.h"
#include <string.h>
#include <math.h>
#include <errno.h>
#include "hisvg-css.h"
#include "hisvg-io.h"

cairo_surface_t *
hisvg_cairo_surface_new_from_href (HiSVGHandle *handle,
                                  const char *href,
                                  GError **error)
{
    return NULL;
}

void
hisvg_preserve_aspect_ratio (unsigned int aspect_ratio, double width,
                            double height, double *w, double *h, double *x, double *y)
{
    double neww, newh;
    if (aspect_ratio & ~HISVG_ASPECT_RATIO_SLICE) {
        neww = *w;
        newh = *h;
        if ((height * *w > width * *h) == ((aspect_ratio & HISVG_ASPECT_RATIO_SLICE) == 0)) {
            neww = width * *h / height;
        } else {
            newh = height * *w / width;
        }

        if (aspect_ratio & HISVG_ASPECT_RATIO_XMIN_YMIN ||
            aspect_ratio & HISVG_ASPECT_RATIO_XMIN_YMID ||
            aspect_ratio & HISVG_ASPECT_RATIO_XMIN_YMAX) {
        } else if (aspect_ratio & HISVG_ASPECT_RATIO_XMID_YMIN ||
                   aspect_ratio & HISVG_ASPECT_RATIO_XMID_YMID ||
                   aspect_ratio & HISVG_ASPECT_RATIO_XMID_YMAX)
            *x -= (neww - *w) / 2;
        else
            *x -= neww - *w;

        if (aspect_ratio & HISVG_ASPECT_RATIO_XMIN_YMIN ||
            aspect_ratio & HISVG_ASPECT_RATIO_XMID_YMIN ||
            aspect_ratio & HISVG_ASPECT_RATIO_XMAX_YMIN) {
        } else if (aspect_ratio & HISVG_ASPECT_RATIO_XMIN_YMID ||
                   aspect_ratio & HISVG_ASPECT_RATIO_XMID_YMID ||
                   aspect_ratio & HISVG_ASPECT_RATIO_XMAX_YMID)
            *y -= (newh - *h) / 2;
        else
            *y -= newh - *h;

        *w = neww;
        *h = newh;
    }
}

static void
hisvg_node_image_free (HiSVGNode * self)
{
    HiSVGNodeImage *z = (HiSVGNodeImage *) self;
    hisvg_state_finalize (z->super.state);
    g_free (z->super.state);
    z->super.state = NULL;
    if (z->surface)
        cairo_surface_destroy (z->surface);
    _hisvg_node_free(self);
}

static void
hisvg_node_image_draw (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate)
{
    HiSVGNodeImage *z = (HiSVGNodeImage *) self;
    unsigned int aspect_ratio = z->preserve_aspect_ratio;
    gdouble x, y, w, h;
    cairo_surface_t *surface = z->surface;

    if (surface == NULL)
        return;

    x = _hisvg_css_normalize_length (&z->x, ctx, 'h');
    y = _hisvg_css_normalize_length (&z->y, ctx, 'v');
    w = _hisvg_css_normalize_length (&z->w, ctx, 'h');
    h = _hisvg_css_normalize_length (&z->h, ctx, 'v');

    hisvg_state_reinherit_top (ctx, z->super.state, dominate);

    hisvg_push_discrete_layer (ctx);

    if (!hisvg_current_state (ctx)->overflow && (aspect_ratio & HISVG_ASPECT_RATIO_SLICE)) {
        hisvg_add_clipping_rect (ctx, x, y, w, h);
    }

    hisvg_preserve_aspect_ratio (aspect_ratio, 
                                (double) cairo_image_surface_get_width (surface),
                                (double) cairo_image_surface_get_height (surface), 
                                &w, &h, &x, &y);

    hisvg_render_surface (ctx, surface, x, y, w, h);

    hisvg_pop_discrete_layer (ctx);
}

static void
hisvg_node_image_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *klazz = NULL, *id = NULL, *value;
    HiSVGNodeImage *image = (HiSVGNodeImage *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "x")))
            image->x = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y")))
            image->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "width")))
            image->w = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "height")))
            image->h = _hisvg_css_parse_length (value);
        /* path is used by some older adobe illustrator versions */
        if ((value = hisvg_property_bag_lookup (atts, "path"))
            || (value = hisvg_property_bag_lookup (atts, "xlink:href"))) {
            image->surface = hisvg_cairo_surface_new_from_href (ctx,
                                                               value, 
                                                               NULL);

            if (!image->surface) {
#ifdef G_ENABLE_DEBUG
                g_warning ("Couldn't load image: %s\n", value);
#endif
            }
        }
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, id, &image->super);
        }
        if ((value = hisvg_property_bag_lookup (atts, "preserveAspectRatio")))
            image->preserve_aspect_ratio = hisvg_css_parse_aspect_ratio (value);

        hisvg_parse_style_attrs (ctx, image->super.state, "image", klazz, id, atts);
    }
}

HiSVGNode *
hisvg_new_image (const char* name)
{
    HiSVGNodeImage *image;
    image = g_new (HiSVGNodeImage, 1);
    _hisvg_node_init (&image->super, HISVG_NODE_TYPE_IMAGE, name);
    g_assert (image->super.state);
    image->surface = NULL;
    image->preserve_aspect_ratio = HISVG_ASPECT_RATIO_XMID_YMID;
    image->x = image->y = image->w = image->h = _hisvg_css_parse_length ("0");
    image->super.free = hisvg_node_image_free;
    image->super.draw = hisvg_node_image_draw;
    image->super.set_atts = hisvg_node_image_set_atts;
    return &image->super;
}
