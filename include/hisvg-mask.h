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

#ifndef HISVG_MASK_H
#define HISVG_MASK_H

#include "hisvg-common.h"
#include "hisvg-defs.h"
#include "hisvg-css.h"
#include "hisvg-styles.h"
#include "hisvg-shapes.h"
#include <libxml/SAX.h>

G_BEGIN_DECLS 

typedef HiSVGCoordUnits HiSVGMaskUnits;

typedef struct _HiSVGMask HiSVGMask;

struct _HiSVGMask {
    HiSVGNode super;
    HiSVGLength x, y, width, height;
    HiSVGMaskUnits maskunits;
    HiSVGMaskUnits contentunits;
};

G_GNUC_INTERNAL
HiSVGNode *hisvg_new_mask	    (const char* name);

typedef struct _HiSVGClipPath HiSVGClipPath;

struct _HiSVGClipPath {
    HiSVGNode super;
    HiSVGCoordUnits units;
};

G_GNUC_INTERNAL
HiSVGNode *hisvg_new_clip_path	(const char* name);

G_END_DECLS
#endif
