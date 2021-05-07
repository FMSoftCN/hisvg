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

#include "hisvg-common.h"
#include "hisvg-io.h"
#include "hisvg-private.h"

static gboolean
hisvg_handle_fill_with_data (HiSVGHandle * handle,
                            const char * data, gsize data_len, GError ** error)
{
    gboolean rv;

    hisvg_return_val_if_fail (data != NULL, FALSE, error);
    hisvg_return_val_if_fail (data_len != 0, FALSE, error);

    rv = hisvg_handle_write (handle, (guchar *) data, data_len, error);

    return hisvg_handle_close (handle, rv ? error : NULL) && rv;
}

/**
 * hisvg_handle_new_from_data:
 * @data: (array length=data_len): The SVG data
 * @data_len: The length of @data, in bytes
 * @error: return location for errors
 *
 * Loads the SVG specified by @data.
 *
 * Returns: A #HiSVGHandle or %NULL if an error occurs.
 * Since: 2.14
 */
HiSVGHandle *
hisvg_handle_new_from_data (const guint8 * data, gsize data_len, GError ** error)
{
    HiSVGHandle *handle;

    handle = hisvg_handle_new (HISVG_HANDLE_FLAGS_NONE);

    if (handle) {
        if (!hisvg_handle_fill_with_data (handle, (char *) data, data_len, error)) {
            g_object_unref (handle);
            handle = NULL;
        }
    }

    return handle;
}

/**
 * hisvg_handle_new_from_file:
 * @file_name: The file name to load. If built with gnome-vfs, can be a URI.
 * @error: return location for errors
 *
 * Loads the SVG specified by @file_name.
 *
 * Returns: A #HiSVGHandle or %NULL if an error occurs.
 * Since: 2.14
 */
HiSVGHandle *
hisvg_handle_new_from_file (const gchar * file_name, GError ** error)
{
    gchar *base_uri;
    char *data;
    gsize data_len;
    HiSVGHandle *handle = NULL;

    hisvg_return_val_if_fail (file_name != NULL, NULL, error);

    base_uri = hisvg_get_base_uri_from_filename (file_name);
    data = _hisvg_io_acquire_data (file_name, base_uri, NULL, &data_len, NULL, error);

    if (data) {
        handle = hisvg_handle_new (HISVG_HANDLE_FLAGS_NONE);
        hisvg_handle_set_base_uri (handle, base_uri);
        if (!hisvg_handle_fill_with_data (handle, data, data_len, error)) {
            g_object_unref (handle);
            handle = NULL;
        }
        g_free (data);
    }

    g_free (base_uri);

    return handle;
}
