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
#ifndef HISVG_CSS_H
#define HISVG_CSS_H

#include <glib.h>

#ifdef HISVG_COMPILATION
#include "hisvg-private.h"
#endif

#include "hisvg-styles.h"

G_BEGIN_DECLS

#define HISVG_ASPECT_RATIO_NONE (0)
#define HISVG_ASPECT_RATIO_XMIN_YMIN (1 << 0)
#define HISVG_ASPECT_RATIO_XMID_YMIN (1 << 1)
#define HISVG_ASPECT_RATIO_XMAX_YMIN (1 << 2)
#define HISVG_ASPECT_RATIO_XMIN_YMID (1 << 3)
#define HISVG_ASPECT_RATIO_XMID_YMID (1 << 4)
#define HISVG_ASPECT_RATIO_XMAX_YMID (1 << 5)
#define HISVG_ASPECT_RATIO_XMIN_YMAX (1 << 6)
#define HISVG_ASPECT_RATIO_XMID_YMAX (1 << 7)
#define HISVG_ASPECT_RATIO_XMAX_YMAX (1 << 8)
#define HISVG_ASPECT_RATIO_SLICE (1 << 31)

/* This one is semi-public for mis-use in hisvg-convert */
guint32	    hisvg_css_parse_color        (const char *str, gboolean * inherit);

#ifdef HISVG_COMPILATION

G_GNUC_INTERNAL
int	    hisvg_css_parse_aspect_ratio	    (const char *str);
G_GNUC_INTERNAL
guint       hisvg_css_parse_opacity	    (const char *str);
G_GNUC_INTERNAL
double      hisvg_css_parse_angle        (const char *str);
G_GNUC_INTERNAL
double      hisvg_css_parse_frequency    (const char *str);
G_GNUC_INTERNAL
double      hisvg_css_parse_time         (const char *str);
G_GNUC_INTERNAL
HiSVGTextStyle   hisvg_css_parse_font_style      (const char *str, gboolean * inherit);
G_GNUC_INTERNAL
HiSVGTextVariant hisvg_css_parse_font_variant    (const char *str, gboolean * inherit);
G_GNUC_INTERNAL
HiSVGTextWeight	 hisvg_css_parse_font_weight	    (const char *str, gboolean * inherit);
G_GNUC_INTERNAL
HiSVGTextStretch hisvg_css_parse_font_stretch    (const char *str, gboolean * inherit);
G_GNUC_INTERNAL
const char  *hisvg_css_parse_font_family	    (const char *str, gboolean * inherit);
G_GNUC_INTERNAL
HiSVGViewBox	  hisvg_css_parse_vbox           (const char *vbox);
G_GNUC_INTERNAL
void          hisvg_css_parse_number_optional_number	(const char *str, double *x, double *y);
G_GNUC_INTERNAL
gchar       **hisvg_css_parse_list           (const char *in_str, guint * out_list_len);
G_GNUC_INTERNAL
gdouble	     *hisvg_css_parse_number_list    (const char *in_str, guint * out_list_len);
G_GNUC_INTERNAL
gboolean      hisvg_css_parse_overflow       (const char *str, gboolean * inherit);
G_GNUC_INTERNAL
char        **hisvg_css_parse_xml_attribute_string   (const char *attribute_string);

#endif /* HISVG_COMPILATION */

G_END_DECLS

#endif  
