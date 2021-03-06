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

#ifndef HISVG_STYLES_H
#define HISVG_STYLES_H

#include <cairo.h>
#include "hisvg-common.h"
#include "hisvg-paint-server.h"
#include "hisvg-text-helper.h"

#include <libxml/SAX.h>

G_BEGIN_DECLS 

typedef int TextDecoration;

#define HISVG_TEXT_GRAVITY_IS_VERTICAL(gravity) \
    ((gravity) == HISVG_TEXT_GRAVITY_EAST || (gravity) == HISVG_TEXT_GRAVITY_WEST)

#define HISVG_TEXT_SCALE 1 //1024

/* enums and data structures are ABI compatible with libart */

typedef struct _HiSVGVpathDash HiSVGVpathDash;

struct _HiSVGVpathDash {
    HiSVGLength offset;
    int n_dash;
    double *dash;
};

/* end libart theft... */

struct _HiSVGState {
    HiSVGState *parent;
    cairo_matrix_t affine;
    cairo_matrix_t personal_affine;

    char *filter;
    char *mask;
    char *clip_path;
    guint8 opacity;             /* 0..255 */
    double baseline_shift;
    gboolean has_baseline_shift;

    HiSVGPaintServer *fill;
    gboolean has_fill_server;
    guint8 fill_opacity;        /* 0..255 */
    gboolean has_fill_opacity;
    gint fill_rule;
    gboolean has_fill_rule;
    gint clip_rule;
    gboolean has_clip_rule;

    gboolean overflow;
    gboolean has_overflow;

    HiSVGPaintServer *stroke;
    gboolean has_stroke_server;
    guint8 stroke_opacity;      /* 0..255 */
    gboolean has_stroke_opacity;
    HiSVGLength stroke_width;
    gboolean has_stroke_width;
    double miter_limit;
    gboolean has_miter_limit;

    cairo_line_cap_t cap;
    gboolean has_cap;
    cairo_line_join_t join;
    gboolean has_join;

    HiSVGLength font_size;
    gboolean has_font_size;
    char *font_family;
    gboolean has_font_family;
    char *lang;
    gboolean has_lang;
    HiSVGTextStyle font_style;
    gboolean has_font_style;
    HiSVGTextVariant font_variant;
    gboolean has_font_variant;
    HiSVGTextWeight font_weight;
    gboolean has_font_weight;
    HiSVGTextStretch font_stretch;
    gboolean has_font_stretch;
    TextDecoration font_decor;
    gboolean has_font_decor;
    HiSVGTextDirection text_dir;
    gboolean has_text_dir;
    HiSVGTextGravity text_gravity;
    gboolean has_text_gravity;
    UnicodeBidi unicode_bidi;
    gboolean has_unicode_bidi;
    TextAnchor text_anchor;
    gboolean has_text_anchor;
    HiSVGLength letter_spacing;
    gboolean has_letter_spacing;
    guint32 writing_mode;

    guint text_offset;

    guint32 stop_color;         /* rgb */
    gboolean has_stop_color;
    gint stop_opacity;          /* 0..255 */
    gboolean has_stop_opacity;

    gboolean visible;
    gboolean has_visible;

    gboolean space_preserve;
    gboolean has_space_preserve;

    gboolean has_cond;
    gboolean cond_true;

    HiSVGVpathDash dash;
    gboolean has_dash;
    gboolean has_dashoffset;

    guint32 current_color;
    gboolean has_current_color;

    guint32 flood_color;
    gboolean has_flood_color;

    guchar flood_opacity;
    gboolean has_flood_opacity;

    char *startMarker;
    char *middleMarker;
    char *endMarker;
    gboolean has_startMarker;
    gboolean has_middleMarker;
    gboolean has_endMarker;

    cairo_operator_t comp_op;
    HiSVGEnableBackgroundType enable_background;

    cairo_antialias_t shape_rendering_type;
    gboolean has_shape_rendering_type;

    cairo_antialias_t text_rendering_type;
    gboolean has_text_rendering_type;

    GHashTable *styles;
    HiSVGNode* node;
};

G_GNUC_INTERNAL
HiSVGState *hisvg_state_new (void);

G_GNUC_INTERNAL
void hisvg_state_init        (HiSVGState * state);
G_GNUC_INTERNAL
void hisvg_state_reinit      (HiSVGState * state);
G_GNUC_INTERNAL
void hisvg_state_clone       (HiSVGState * dst, const HiSVGState * src);
G_GNUC_INTERNAL
void hisvg_state_inherit     (HiSVGState * dst, const HiSVGState * src);
G_GNUC_INTERNAL
void hisvg_state_reinherit   (HiSVGState * dst, const HiSVGState * src);
G_GNUC_INTERNAL
void hisvg_state_dominate    (HiSVGState * dst, const HiSVGState * src);
G_GNUC_INTERNAL
void hisvg_state_override    (HiSVGState * dst, const HiSVGState * src);
G_GNUC_INTERNAL
void hisvg_state_finalize    (HiSVGState * state);
G_GNUC_INTERNAL
void hisvg_state_free_all    (HiSVGState * state);

/* VW: to override the author style */
G_GNUC_INTERNAL
void hisvg_parse_style_pair (HiSVGHandle * ctx, HiSVGState * state,
                       const gchar * name, const gchar * value, gboolean important);
G_GNUC_INTERNAL
void hisvg_parse_style_pairs (HiSVGHandle * ctx, HiSVGState * state, HiSVGPropertyBag * atts);
G_GNUC_INTERNAL
void hisvg_parse_style	    (HiSVGHandle * ctx, HiSVGState * state, const char *str);
G_GNUC_INTERNAL
void hisvg_parse_cssbuffer   (HiSVGHandle * ctx, const char *buff, size_t buflen);
G_GNUC_INTERNAL
void hisvg_parse_style_attrs (HiSVGHandle * ctx, HiSVGState * state, const char *tag,
                             const char *klazz, const char *id, HiSVGPropertyBag * atts);

G_GNUC_INTERNAL
gdouble hisvg_viewport_percentage (gdouble width, gdouble height);
G_GNUC_INTERNAL
gdouble hisvg_dpi_percentage      (HiSVGHandle * ctx);

G_GNUC_INTERNAL
gboolean hisvg_parse_transform   (cairo_matrix_t *matrix, const char *src);

G_GNUC_INTERNAL
HiSVGState *hisvg_state_parent    (HiSVGState * state);

G_GNUC_INTERNAL
void       hisvg_state_pop       (HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
void       hisvg_state_push      (HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
HiSVGState *hisvg_current_state   (HiSVGDrawingCtx * ctx);

G_GNUC_INTERNAL
void hisvg_state_reinherit_top	(HiSVGDrawingCtx * ctx, HiSVGState * state, int dominate);

G_GNUC_INTERNAL
void hisvg_state_reconstruct	(HiSVGState * state, HiSVGNode * current);

G_END_DECLS

#endif                          /* HISVG_STYLES_H */
