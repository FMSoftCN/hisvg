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

#ifndef _HI_SVG_SELECT_H_
#define _HI_SVG_SELECT_H_

#include "hisvg-common.h"
#include "hisvg-private.h"
#include "hisvg-css.h"

#define CSS_SELECT_FUNC(pname)                                           \
  int32_t css_select_##pname (HiSVGHandle* handle, HiSVGNode* node, HiSVGState* state, HLUsedSvgValues* svg_value)


enum hisvg_css_select_prop_e {
    HISVG_CSS_PROP_BASELINE_SHIFT,
    HISVG_CSS_PROP_CLIP_PATH,
    HISVG_CSS_PROP_CLIP_RULE,
    HISVG_CSS_PROP_COLOR,
    HISVG_CSS_PROP_DIRECTION,
    HISVG_CSS_PROP_DISPLAY,
    HISVG_CSS_PROP_ENABLE_BACKGROUND,
    HISVG_CSS_PROP_COMP_OP,
    HISVG_CSS_PROP_FILL,
    HISVG_CSS_PROP_FILL_OPACITY,
    HISVG_CSS_PROP_FILL_RULE,
    HISVG_CSS_PROP_FILTER,
    HISVG_CSS_PROP_FLOOD_COLOR,
    HISVG_CSS_PROP_FLOOD_OPACITY,
    HISVG_CSS_PROP_FONT_FAMILY,
    HISVG_CSS_PROP_FONT_SIZE,
    HISVG_CSS_PROP_FONT_STRETCH,
    HISVG_CSS_PROP_FONT_STYLE,
    HISVG_CSS_PROP_FONT_VARIANT,
    HISVG_CSS_PROP_FONT_WEIGHT,
    HISVG_CSS_PROP_MARKER_END,
    HISVG_CSS_PROP_MASK,
    HISVG_CSS_PROP_MARKER_MID,
    HISVG_CSS_PROP_MARKER_START,
    HISVG_CSS_PROP_OPACITY,
    HISVG_CSS_PROP_OVERFLOW,
    HISVG_CSS_PROP_SHAPE_RENDERING,
    HISVG_CSS_PROP_TEXT_RENDERING,
    HISVG_CSS_PROP_STOP_COLOR,
    HISVG_CSS_PROP_STOP_OPACITY,
    HISVG_CSS_PROP_STROKE,
    HISVG_CSS_PROP_STROKE_DASHARRAY,
    HISVG_CSS_PROP_STROKE_DASHOFFSET,
    HISVG_CSS_PROP_STROKE_LINECAP,
    HISVG_CSS_PROP_STROKE_LINEJOIN,
    HISVG_CSS_PROP_STROKE_MITERLIMIT,
    HISVG_CSS_PROP_STROKE_OPACITY,
    HISVG_CSS_PROP_STROKE_WIDTH,
    HISVG_CSS_PROP_TEXT_ANCHOR,
    HISVG_CSS_PROP_TEXT_DECORATION,
    HISVG_CSS_PROP_UNICODE_BIDI,
    HISVG_CSS_PROP_LETTER_SPACING,
    HISVG_CSS_PROP_VISIBILITY,
    HISVG_CSS_PROP_WRITING_MODE,

    HISVG_CSS_PROP_N
};

extern struct css_select_prop_table {
    int32_t (*css_select) (HiSVGHandle* handle, 
            HiSVGNode* node, HiSVGState* state,
            HLUsedSvgValues* svg_value);
} css_select_prop_dispatch[HISVG_CSS_PROP_N];

CSS_SELECT_FUNC(baseline_shift);
CSS_SELECT_FUNC(clip_path);
CSS_SELECT_FUNC(clip_rule);
CSS_SELECT_FUNC(color);
CSS_SELECT_FUNC(direction);
CSS_SELECT_FUNC(display);
CSS_SELECT_FUNC(enable_background);
CSS_SELECT_FUNC(comp_op);
CSS_SELECT_FUNC(fill);
CSS_SELECT_FUNC(fill_opacity);
CSS_SELECT_FUNC(fill_rule);
CSS_SELECT_FUNC(filter);
CSS_SELECT_FUNC(flood_color);
CSS_SELECT_FUNC(flood_opacity);
CSS_SELECT_FUNC(font_family);
CSS_SELECT_FUNC(font_size);
CSS_SELECT_FUNC(font_stretch);
CSS_SELECT_FUNC(font_style);
CSS_SELECT_FUNC(font_variant);
CSS_SELECT_FUNC(font_weight);
CSS_SELECT_FUNC(marker_end);
CSS_SELECT_FUNC(mask);
CSS_SELECT_FUNC(marker_mid);
CSS_SELECT_FUNC(marker_start);
CSS_SELECT_FUNC(opacity);
CSS_SELECT_FUNC(overflow);
CSS_SELECT_FUNC(shape_rendering);
CSS_SELECT_FUNC(text_rendering);
CSS_SELECT_FUNC(stop_color);
CSS_SELECT_FUNC(stop_opacity);
CSS_SELECT_FUNC(stroke);
CSS_SELECT_FUNC(stroke_dasharray);
CSS_SELECT_FUNC(stroke_dashoffset);
CSS_SELECT_FUNC(stroke_linecap);
CSS_SELECT_FUNC(stroke_linejoin);
CSS_SELECT_FUNC(stroke_miterlimit);
CSS_SELECT_FUNC(stroke_opacity);
CSS_SELECT_FUNC(stroke_width);
CSS_SELECT_FUNC(text_anchor);
CSS_SELECT_FUNC(text_decoration);
CSS_SELECT_FUNC(unicode_bidi);
CSS_SELECT_FUNC(letter_spacing);
CSS_SELECT_FUNC(visibility);
CSS_SELECT_FUNC(writing_mode);

#endif // _HI_SVG_SELECT_H_
