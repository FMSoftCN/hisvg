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

    Copyright (C) 2004 Caleb Moore
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
#include "hisvg-mask.h"
#include "hisvg-styles.h"
#include "hisvg-css.h"
#include <string.h>

static void
hisvg_mask_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *id = NULL, *klazz = NULL, *value;
    HiSVGMask *mask;
    mask = (HiSVGMask *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "maskUnits"))) {
            if (!strcmp (value, "userSpaceOnUse"))
                mask->maskunits = userSpaceOnUse;
            else
                mask->maskunits = objectBoundingBox;
        }
        if ((value = hisvg_property_bag_lookup (atts, "maskContentUnits"))) {
            if (!strcmp (value, "objectBoundingBox"))
                mask->contentunits = objectBoundingBox;
            else
                mask->contentunits = userSpaceOnUse;
        }
        if ((value = hisvg_property_bag_lookup (atts, "x")))
            mask->x = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "y")))
            mask->y = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "width")))
            mask->width = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "height")))
            mask->height = _hisvg_css_parse_length (value);
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, id, &mask->super);
        }
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
    }

    hisvg_parse_style_attrs (ctx, mask->super.state, "mask", klazz, id, atts);
}

HiSVGNode *
hisvg_new_mask (const char* name)
{
    HiSVGMask *mask;

    mask = g_new (HiSVGMask, 1);
    _hisvg_node_init (&mask->super, HISVG_NODE_TYPE_MASK, name);
    mask->maskunits = objectBoundingBox;
    mask->contentunits = userSpaceOnUse;
    mask->x = _hisvg_css_parse_length ("0");
    mask->y = _hisvg_css_parse_length ("0");
    mask->width = _hisvg_css_parse_length ("1");
    mask->height = _hisvg_css_parse_length ("1");
    mask->super.set_atts = hisvg_mask_set_atts;
    return &mask->super;
}

char *
hisvg_get_url_string (const char *str)
{
    if (!strncmp (str, "url(", 4)) {
        const char *p = str + 4;
        int ix;

        while (g_ascii_isspace (*p))
            p++;

        for (ix = 0; p[ix]; ix++)
            if (p[ix] == ')')
                return g_strndup (p, ix);
    }
    return NULL;
}

static void
hisvg_clip_path_set_atts (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    const char *id = NULL, *klazz = NULL, *value = NULL;
    HiSVGClipPath *clip_path;

    clip_path = (HiSVGClipPath *) self;

    if (hisvg_property_bag_size (atts)) {
        if ((value = hisvg_property_bag_lookup (atts, "clipPathUnits"))) {
            if (!strcmp (value, "objectBoundingBox"))
                clip_path->units = objectBoundingBox;
            else
                clip_path->units = userSpaceOnUse;
        }
        if ((value = hisvg_property_bag_lookup (atts, "id"))) {
            HISVG_NODE_SET_ID(self, value);
            id = value;
            hisvg_defs_register_name (ctx->priv->defs, id, &clip_path->super);
        }
        if ((value = hisvg_property_bag_lookup (atts, "class"))) {
            HISVG_NODE_INCLUDE_CLASS(self, value);
            klazz = value;
        }
    }

    hisvg_parse_style_attrs (ctx, clip_path->super.state, "clipPath", klazz, id, atts);
}

HiSVGNode *
hisvg_new_clip_path (const char* name)
{
    HiSVGClipPath *clip_path;

    clip_path = g_new (HiSVGClipPath, 1);
    _hisvg_node_init (&clip_path->super, HISVG_NODE_TYPE_CLIP_PATH, name);
    clip_path->units = userSpaceOnUse;
    clip_path->super.set_atts = hisvg_clip_path_set_atts;
    clip_path->super.free = _hisvg_node_free;
    return &clip_path->super;
}
