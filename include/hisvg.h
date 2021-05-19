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


#ifndef _HI_SVG_H_
#define _HI_SVG_H_

#include <glib.h>
#include <glib/gquark.h>
#include <cairo.h>
#include "hidomlayout.h"

typedef struct _HiSVGHandle HiSVGHandle;

typedef enum _HiSVGHandleFlags
{
    // default none
    HISVG_HANDLE_FLAGS_NONE           = 0,
    // Allow any SVG XML without size limitations.
    HISVG_HANDLE_FLAG_UNLIMITED       = 1 << 0,
    // Keeps the image data when for use by cairo.
    HISVG_HANDLE_FLAG_KEEP_IMAGE_DATA = 1 << 1
} HiSVGHandleFlags;

typedef struct _HiSVGLength {
    double length;
    char factor;
} HiSVGLength;

typedef struct _HiSVGRect {
    double x;
    double y;
    double width;
    double height;
} HiSVGRect;


typedef struct _HiSVGDimension {
    uint8_t has_w;
    uint8_t has_h;
    uint8_t has_vbox;
    HiSVGLength w;
    HiSVGLength h;
    HiSVGRect vbox;
} HiSVGDimension;

#ifdef __cplusplus
extern "C" {
#endif


#if 0
#define HISVG_ERROR (hisvg_error_quark ())
GQuark hisvg_error_quark (void) G_GNUC_CONST;
#endif

HiSVGHandle* hisvg_handle_new (HiSVGHandleFlags flags);
void hisvg_handle_destroy (HiSVGHandle* handle);

void hisvg_handle_set_dpi (HiSVGHandle* handle, double dpi_x, double dpi_y);

void hisvg_handle_set_base_uri (HiSVGHandle* handle, const char* base_uri);
const char* hisvg_handle_get_base_uri (HiSVGHandle* handle);
gboolean hisvg_handle_has_sub (HiSVGHandle* handle, const char* id);
HiSVGHandle* hisvg_handle_new_from_data (const guint8* data, gsize data_len, GError** error);
HiSVGHandle* hisvg_handle_new_from_file (const gchar* file_name, GError** error);

gboolean hisvg_handle_render_cairo (HiSVGHandle* handle, cairo_t* cr, const HiSVGRect* viewport, const char* id, GError** error);
HLDomElementNode* hisvg_handle_get_node (HiSVGHandle* handle, const char* id);
gboolean hisvg_handle_set_stylesheet (HiSVGHandle* handle, const char* id, const guint8* css, gsize css_len, GError** error);
void hisvg_handle_get_dimensions (HiSVGHandle* handle, HiSVGDimension* dimension);

#ifdef __cplusplus
}
#endif

#endif // _HI_SVG_H_
