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

#include "hisvg-private.h"
#include "hisvg-defs.h"
#include "hisvg-styles.h"
#include "hisvg-io.h"

#include <glib.h>

struct _HiSVGDefs {
    GHashTable *hash;
    GPtrArray *unnamed;
    GHashTable *externs;
    HiSVGHandle *ctx;
};

HiSVGDefs *
hisvg_defs_new (HiSVGHandle *handle)
{
    HiSVGDefs *result = g_new (HiSVGDefs, 1);

    result->hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    result->externs =
        g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_object_unref);
    result->unnamed = g_ptr_array_new ();
    result->ctx = handle; /* no need to take a ref here */

    return result;
}

static int
hisvg_defs_load_extern (const HiSVGDefs * defs, const char *name)
{
    HiSVGHandle *handle;
    gchar *filename, *base_uri;
    char *data;
    gsize data_len;
    gboolean rv;

    filename = _hisvg_io_get_file_path (name, hisvg_handle_get_base_uri (defs->ctx));

    data = _hisvg_handle_acquire_data (defs->ctx, name, NULL, &data_len, NULL);

    if (data) {
        handle = hisvg_handle_new (HISVG_HANDLE_FLAGS_NONE);

        base_uri = hisvg_get_base_uri_from_filename (filename);
        hisvg_handle_set_base_uri (handle, base_uri);
        g_free (base_uri);

        rv = hisvg_handle_write (handle, (guchar *) data, data_len, NULL);
        rv = hisvg_handle_close (handle, NULL) && rv;
        if (rv) {
            g_hash_table_insert (defs->externs, g_strdup (name), handle);
        }

        g_free (data);
    }

    g_free (filename);
    return 0;
}

static HiSVGNode *
hisvg_defs_extern_lookup (const HiSVGDefs * defs, const char *filename, const char *name)
{
    HiSVGHandle *file;
    file = (HiSVGHandle *) g_hash_table_lookup (defs->externs, filename);
    if (file == NULL) {
        if (hisvg_defs_load_extern (defs, filename))
            return NULL;
        file = (HiSVGHandle *) g_hash_table_lookup (defs->externs, filename);
    }

    if (file != NULL)
        return g_hash_table_lookup (file->priv->defs->hash, name);
    else
        return NULL;
}

HiSVGNode *
hisvg_defs_lookup (const HiSVGDefs * defs, const char *name)
{
    char *hashpos;
    hashpos = g_strrstr (name, "#");
    if (!hashpos) {
        return g_hash_table_lookup (defs->hash, name);
    }
    if (hashpos == name) {
        return g_hash_table_lookup (defs->hash, name + 1);
    } else {
        gchar **splitbits;
        HiSVGNode *toreturn;
        splitbits = g_strsplit (name, "#", 2);
        toreturn = hisvg_defs_extern_lookup (defs, splitbits[0], splitbits[1]);
        g_strfreev (splitbits);
        return toreturn;
    }
}

void
hisvg_defs_set (HiSVGDefs * defs, const char *name, HiSVGNode * val)
{
    if (name == NULL);
    else if (name[0] == '\0');
    else
        hisvg_defs_register_name (defs, name, val);
    hisvg_defs_register_memory (defs, val);
}

void
hisvg_defs_register_name (HiSVGDefs * defs, const char *name, HiSVGNode * val)
{
    if (g_hash_table_lookup (defs->hash, name))
        return;

    g_hash_table_insert (defs->hash, g_strdup (name), val);
}

void
hisvg_defs_register_memory (HiSVGDefs * defs, HiSVGNode * val)
{
    g_ptr_array_add (defs->unnamed, val);
}

void
hisvg_defs_free (HiSVGDefs * defs)
{
    guint i;

    g_hash_table_destroy (defs->hash);

    for (i = 0; i < defs->unnamed->len; i++)
        ((HiSVGNode *) g_ptr_array_index (defs->unnamed, i))->
            free (g_ptr_array_index (defs->unnamed, i));
    g_ptr_array_free (defs->unnamed, TRUE);

    g_hash_table_destroy (defs->externs);

    g_free (defs);
}

