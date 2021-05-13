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
    Copyright (C) 2002, 2003, 2004, 2005 Dom Lachowicz <cinamod@hotmail.com>
    Copyright (C) 2003, 2004, 2005 Caleb Moore <c.moore@student.unsw.edu.au>
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

#ifndef HISVG_STRUCTURE_H
#define HISVG_STRUCTURE_H

#include "hisvg-private.h"
#include "hisvg-defs.h"
#include "hisvg-styles.h"

G_BEGIN_DECLS 

G_GNUC_INTERNAL
HiSVGNode *hisvg_new_use (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_symbol (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_svg (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_defs (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_group (const char* name);
G_GNUC_INTERNAL
HiSVGNode *hisvg_new_switch (const char* name);

typedef struct _HiSVGNodeGroup HiSVGNodeGroup;
typedef struct _HiSVGNodeUse HiSVGNodeUse;
typedef struct _HiSVGNodeSymbol HiSVGNodeSymbol;
typedef struct _HiSVGNodeSvg HiSVGNodeSvg;

struct _HiSVGNodeGroup {
    HiSVGNode super;
    char *name;
};

struct _HiSVGNodeSymbol {
    HiSVGNode super;
    gint preserve_aspect_ratio;
    HiSVGViewBox vbox;
};

struct _HiSVGNodeUse {
    HiSVGNode super;
    char *link;
    HiSVGLength x, y, w, h;
};

struct _HiSVGNodeSvg {
    HiSVGNode super;
    gint preserve_aspect_ratio;
    HiSVGLength x, y, w, h;
    HiSVGViewBox vbox;
    HiSVGPropertyBag *atts;
    uint8_t has_w;
    uint8_t has_h;
    uint8_t has_vbox;
};

G_GNUC_INTERNAL
void hisvg_pop_def_group     (HiSVGHandle * ctx);
G_GNUC_INTERNAL
void hisvg_node_group_pack   (HiSVGNode * self, HiSVGNode * child);
G_GNUC_INTERNAL
void hisvg_node_draw         (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate);
G_GNUC_INTERNAL
void _hisvg_node_draw_children   (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate);
G_GNUC_INTERNAL
void _hisvg_node_finalize    (HiSVGNode * self);
G_GNUC_INTERNAL
void _hisvg_node_free        (HiSVGNode * self);
G_GNUC_INTERNAL
void _hisvg_node_init        (HiSVGNode * self, HiSVGNodeType type, const char* name);
G_GNUC_INTERNAL
void _hisvg_node_svg_apply_atts  (HiSVGNodeSvg * self, HiSVGHandle * ctx);

G_END_DECLS

#endif                          /* HISVG_STRUCTURE_H */
