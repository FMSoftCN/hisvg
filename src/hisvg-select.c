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
#include "hisvg-select.h"

#define CSS_SELECT_PROP_FUNC(pname)  css_select_##pname 

struct css_select_prop_table css_select_prop_dispatch[HISVG_CSS_PROP_N] = {
    CSS_SELECT_PROP_FUNC(baseline_shift),
    CSS_SELECT_PROP_FUNC(clip_path),
    CSS_SELECT_PROP_FUNC(clip_rule),
    CSS_SELECT_PROP_FUNC(color),
    CSS_SELECT_PROP_FUNC(direction),
    CSS_SELECT_PROP_FUNC(display),
    CSS_SELECT_PROP_FUNC(enable_background),
    CSS_SELECT_PROP_FUNC(comp_op),
    CSS_SELECT_PROP_FUNC(fill),
    CSS_SELECT_PROP_FUNC(fill_opacity),
    CSS_SELECT_PROP_FUNC(fill_rule),
    CSS_SELECT_PROP_FUNC(filter),
    CSS_SELECT_PROP_FUNC(flood_color),
    CSS_SELECT_PROP_FUNC(flood_opacity),
    CSS_SELECT_PROP_FUNC(font_family),
    CSS_SELECT_PROP_FUNC(font_size),
    CSS_SELECT_PROP_FUNC(font_stretch),
    CSS_SELECT_PROP_FUNC(font_style),
    CSS_SELECT_PROP_FUNC(font_variant),
    CSS_SELECT_PROP_FUNC(font_weight),
    CSS_SELECT_PROP_FUNC(marker_end),
    CSS_SELECT_PROP_FUNC(mask),
    CSS_SELECT_PROP_FUNC(marker_mid),
    CSS_SELECT_PROP_FUNC(marker_start),
    CSS_SELECT_PROP_FUNC(opacity),
    CSS_SELECT_PROP_FUNC(overflow),
    CSS_SELECT_PROP_FUNC(shape_rendering),
    CSS_SELECT_PROP_FUNC(text_rendering),
    CSS_SELECT_PROP_FUNC(stop_color),
    CSS_SELECT_PROP_FUNC(stop_opacity),
    CSS_SELECT_PROP_FUNC(stroke),
    CSS_SELECT_PROP_FUNC(stroke_dasharray),
    CSS_SELECT_PROP_FUNC(stroke_dashoffset),
    CSS_SELECT_PROP_FUNC(stroke_linecap),
    CSS_SELECT_PROP_FUNC(stroke_linejoin),
    CSS_SELECT_PROP_FUNC(stroke_miterlimit),
    CSS_SELECT_PROP_FUNC(stroke_opacity),
    CSS_SELECT_PROP_FUNC(stroke_width),
    CSS_SELECT_PROP_FUNC(text_anchor),
    CSS_SELECT_PROP_FUNC(text_decoration),
    CSS_SELECT_PROP_FUNC(unicode_bidi),
    CSS_SELECT_PROP_FUNC(letter_spacing),
    CSS_SELECT_PROP_FUNC(visibility),
    CSS_SELECT_PROP_FUNC(writing_mode)
};

