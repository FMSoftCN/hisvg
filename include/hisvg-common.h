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

#ifndef HISVG_H
#define HISVG_H

#define __HISVG_HISVG_H_INSIDE__

#include <glib-object.h>
#include <gio/gio.h>
#include "hisvg.h"


G_BEGIN_DECLS

#if defined(HISVG_DISABLE_DEPRECATION_WARNINGS) || !GLIB_CHECK_VERSION (2, 31, 0)
#define HISVG_DEPRECATED
#define HISVG_DEPRECATED_FOR(f)
#else
#define HISVG_DEPRECATED G_DEPRECATED
#define HISVG_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f)
#endif

#define HISVG_TYPE_HANDLE                  (hisvg_handle_get_type ())
#define HISVG_HANDLE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HISVG_TYPE_HANDLE, HiSVGHandle))
#define HISVG_HANDLE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HISVG_TYPE_HANDLE, HiSVGHandleClass))
#define HISVG_IS_HANDLE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HISVG_TYPE_HANDLE))
#define HISVG_IS_HANDLE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HISVG_TYPE_HANDLE))
#define HISVG_HANDLE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HISVG_TYPE_HANDLE, HiSVGHandleClass))

GType hisvg_handle_get_type (void);

/**
 * HiSVGError:
 * @HISVG_ERROR_FAILED: the request failed
 *
 * An enumeration representing possible errors
 */
typedef enum {
    HISVG_ERROR_FAILED
} HiSVGError;


/**
 * HiSVGHandle:
 *
 * The #HiSVGHandle is an object representing the parsed form of a SVG
 */
typedef struct _HiSVGHandle HiSVGHandle;
typedef struct HiSVGHandlePrivate HiSVGHandlePrivate;
typedef struct _HiSVGHandleClass HiSVGHandleClass;
typedef struct _HiSVGDimensionData HiSVGDimensionData;
typedef struct _HiSVGPositionData HiSVGPositionData;

/**
 * HiSVGHandleClass:
 * @parent: parent class
 *
 * Class structure for #HiSVGHandle
 */
struct _HiSVGHandleClass {
    GObjectClass parent;

    /*< private >*/
    gpointer _abi_padding[15];
};

struct _HiSVGHandle {
    GObject parent;

    /*< private >*/

    HiSVGHandlePrivate *priv;

    gpointer _abi_padding[15];
};

/**
 * HiSVGDimensionData:
 * @width: SVG's width, in pixels
 * @height: SVG's height, in pixels
 * @em: em
 * @ex: ex
 */
struct _HiSVGDimensionData {
    int width;
    int height;
    gdouble em;
    gdouble ex;
};

/**
 * HiSVGPositionData:
 * @x: position on the x axis
 * @y: position on the y axis
 *
 * Position of an SVG fragment.
 */
struct _HiSVGPositionData {
    int x;
    int y;
};

void hisvg_cleanup (void);

gboolean     hisvg_handle_write		(HiSVGHandle * handle, const guchar * buf, 
                                     gsize count, GError ** error);
gboolean     hisvg_handle_close		(HiSVGHandle * handle, GError ** error);

void hisvg_handle_get_dimensions_x (HiSVGHandle * handle, HiSVGDimensionData * dimension_data);

gboolean hisvg_handle_get_dimensions_sub (HiSVGHandle * handle, HiSVGDimensionData * dimension_data, const char *id);
gboolean hisvg_handle_get_position_sub (HiSVGHandle * handle, HiSVGPositionData * position_data, const char *id);

void        hisvg_handle_set_base_gfile (HiSVGHandle *handle,
                                        GFile      *base_file);

gboolean    hisvg_handle_read_stream_sync (HiSVGHandle   *handle,
                                          GInputStream *stream,
                                          GCancellable *cancellable,
                                          GError      **error);

HiSVGHandle *hisvg_handle_new_from_gfile_sync (GFile          *file,
                                             HiSVGHandleFlags flags,
                                             GCancellable   *cancellable,
                                             GError        **error);

HiSVGHandle *hisvg_handle_new_from_stream_sync (GInputStream   *input_stream,
                                              GFile          *base_file,
                                              HiSVGHandleFlags flags,
                                              GCancellable   *cancellable,
                                              GError        **error);

G_END_DECLS

//#include "hisvg-enum-types.h"

#undef __HISVG_HISVG_H_INSIDE__

#endif                          /* HISVG_H */
