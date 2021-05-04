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

#include "hisvg-io.h"
#include "hisvg-private.h"

#include <string.h>

/* Copied from soup-request-data.c (LGPL2+):
 * Copyright (C) 2009, 2010 Red Hat, Inc.
 * Copyright (C) 2010 Igalia, S.L.
 * and from soup-uri.c:
 * Copyright 1999-2003 Ximian, Inc.
 */

#define XDIGIT(c) ((c) <= '9' ? (c) - '0' : ((c) & 0x4F) - 'A' + 10)
#define HEXCHAR(s) ((XDIGIT (s[1]) << 4) + XDIGIT (s[2]))

static char *
uri_decoded_copy (const char *part, 
                  gsize length)
{
    unsigned char *s, *d;
    char *decoded = g_strndup (part, length);

    s = d = (unsigned char *)decoded;
    do {
        if (*s == '%') {
            if (!g_ascii_isxdigit (s[1]) ||
                !g_ascii_isxdigit (s[2])) {
                *d++ = *s;
                continue;
            }
            *d++ = HEXCHAR (s);
            s += 2;
        } else {
            *d++ = *s;
        }
    } while (*s++);

    return decoded;
}

#define BASE64_INDICATOR     ";base64"
#define BASE64_INDICATOR_LEN (sizeof (";base64") - 1)

static char *
hisvg_acquire_data_data (const char *uri,
                        const char *base_uri, 
                        char **out_mime_type,
                        gsize *out_len,
                        GError **error)
{
    const char *comma, *start, *end;
    char *mime_type;
    char *data;
    gsize data_len;
    gboolean base64 = FALSE;

    g_assert (out_len != NULL);
    g_assert (strncmp (uri, "data:", 5) == 0);

    mime_type = NULL;
    start = uri + 5;
    comma = strchr (start, ',');

    if (comma && comma != start) {
        /* Deal with MIME type / params */
        if (comma > start + BASE64_INDICATOR_LEN && 
            !g_ascii_strncasecmp (comma - BASE64_INDICATOR_LEN, BASE64_INDICATOR, BASE64_INDICATOR_LEN)) {
            end = comma - BASE64_INDICATOR_LEN;
            base64 = TRUE;
        } else {
            end = comma;
        }

        if (end != start) {
            mime_type = uri_decoded_copy (start, end - start);
        }
    }

    if (comma)
        start = comma + 1;

    if (*start) {
	data = uri_decoded_copy (start, strlen (start));

        if (base64)
            data = (char *) g_base64_decode_inplace (data, &data_len);
        else
            data_len = strlen (data);
    } else {
        data = NULL;
        data_len = 0;
    }

    if (out_mime_type)
        *out_mime_type = mime_type;
    else
        g_free (mime_type);

    *out_len = data_len;
    return data;
}

gchar *
_hisvg_io_get_file_path (const gchar * filename,
                        const gchar * base_uri)
{
    gchar *absolute_filename;

    if (g_file_test (filename, G_FILE_TEST_EXISTS) || g_path_is_absolute (filename)) {
        absolute_filename = g_strdup (filename);
    } else {
        gchar *tmpcdir;
        gchar *base_filename;

        if (base_uri) {
            base_filename = g_filename_from_uri (base_uri, NULL, NULL);
            if (base_filename != NULL) {
                tmpcdir = g_path_get_dirname (base_filename);
                g_free (base_filename);
            } else 
                return NULL;
        } else
            tmpcdir = g_get_current_dir ();

        absolute_filename = g_build_filename (tmpcdir, filename, NULL);
        g_free (tmpcdir);
    }

    return absolute_filename;
}

static char *
hisvg_acquire_file_data (const char *filename,
                        const char *base_uri,
                        char **out_mime_type,
                        gsize *out_len,
                        GCancellable *cancellable,
                        GError **error)
{
    gchar *path, *data;
    gsize len;
    char *content_type;

    hisvg_return_val_if_fail (filename != NULL, NULL, error);
    g_assert (out_len != NULL);

    path = _hisvg_io_get_file_path (filename, base_uri);
    if (path == NULL)
        return NULL;

    if (!g_file_get_contents (path, &data, &len, error)) {
        g_free (path);
        return NULL;
    }

    if (out_mime_type &&
        (content_type = g_content_type_guess (path, (guchar *) data, len, NULL))) {
        *out_mime_type = g_content_type_get_mime_type (content_type);
        g_free (content_type);
    }

    g_free (path);

    *out_len = len;
    return data;
}

static GInputStream *
hisvg_acquire_gvfs_stream (const char *uri, 
                          const char *base_uri, 
                          char **out_mime_type,
                          GCancellable *cancellable,
                          GError **error)
{
    GFile *base, *file;
    GFileInputStream *stream;
    GError *err = NULL;

    file = g_file_new_for_uri (uri);

    stream = g_file_read (file, cancellable, &err);
    g_object_unref (file);

    if (stream == NULL &&
        g_error_matches (err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
        g_clear_error (&err);

        base = g_file_new_for_uri (base_uri);
        file = g_file_resolve_relative_path (base, uri);
        g_object_unref (base);

        stream = g_file_read (file, cancellable, &err);
        g_object_unref (file);
    }

    if (stream == NULL) {
        g_propagate_error (error, err);
        return NULL;
    }

    if (out_mime_type) {
        GFileInfo *file_info;
        const char *content_type;

        file_info = g_file_input_stream_query_info (stream, 
                                                    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                                    cancellable,
                                                    NULL /* error */);
        if (file_info &&
            (content_type = g_file_info_get_content_type (file_info)))
            *out_mime_type = g_content_type_get_mime_type (content_type);
        else
            *out_mime_type = NULL;

        if (file_info)
            g_object_unref (file_info);
    }

    return G_INPUT_STREAM (stream);
}

static char *
hisvg_acquire_gvfs_data (const char *uri,
                        const char *base_uri,
                        char **out_mime_type,
                        gsize *out_len,
                        GCancellable *cancellable,
                        GError **error)
{
    GFile *base, *file;
    GError *err;
    char *data;
    gsize len;
    char *content_type;
    gboolean res;

    file = g_file_new_for_uri (uri);

    err = NULL;
    data = NULL;
    if (!(res = g_file_load_contents (file, cancellable, &data, &len, NULL, &err)) &&
        g_error_matches (err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND) &&
        base_uri != NULL) {
        g_clear_error (&err);
        g_object_unref (file);

        base = g_file_new_for_uri (base_uri);
        file = g_file_resolve_relative_path (base, uri);
        g_object_unref (base);

        res = g_file_load_contents (file, cancellable, &data, &len, NULL, &err);
    }

    g_object_unref (file);

    if (err) {
        g_propagate_error (error, err);
        return NULL;
    }

    if (out_mime_type &&
        (content_type = g_content_type_guess (uri, (guchar *) data, len, NULL))) {
        *out_mime_type = g_content_type_get_mime_type (content_type);
        g_free (content_type);
    }

    *out_len = len;
    return data;
}

char *
_hisvg_io_acquire_data (const char *href, 
                       const char *base_uri, 
                       char **mime_type,
                       gsize *len,
                       GCancellable *cancellable,
                       GError **error)
{
    char *data;
    gsize llen;

    if (!(href && *href)) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                            "Invalid URI");
        return NULL;
    }

    if (!len)
        len = &llen;

    if (strncmp (href, "data:", 5) == 0)
      return hisvg_acquire_data_data (href, NULL, mime_type, len, error);

    if ((data = hisvg_acquire_file_data (href, base_uri, mime_type, len, cancellable, NULL)))
      return data;

    if ((data = hisvg_acquire_gvfs_data (href, base_uri, mime_type, len, cancellable, error)))
      return data;

    return NULL;
}

GInputStream *
_hisvg_io_acquire_stream (const char *href, 
                         const char *base_uri, 
                         char **mime_type,
                         GCancellable *cancellable,
                         GError **error)
{
    GInputStream *stream;
    char *data;
    gsize len;

    if (!(href && *href)) {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                            "Invalid URI");
        return NULL;
    }

    if (strncmp (href, "data:", 5) == 0) {
        if (!(data = hisvg_acquire_data_data (href, NULL, mime_type, &len, error)))
            return NULL;

        return g_memory_input_stream_new_from_data (data, len, (GDestroyNotify) g_free);
    }

    if ((data = hisvg_acquire_file_data (href, base_uri, mime_type, &len, cancellable, NULL)))
      return g_memory_input_stream_new_from_data (data, len, (GDestroyNotify) g_free);

    if ((stream = hisvg_acquire_gvfs_stream (href, base_uri, mime_type, cancellable, error)))
      return stream;

    return NULL;
}
