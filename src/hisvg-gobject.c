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

    Copyright (C) 2006 Robert Staudinger <robert.staudinger@gmail.com>
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

/**
 * SECTION: hisvg-handle
 * @short_description: Create and manipulate SVG objects
 *
 * libhisvg is a component used within software applications to enable
 * support for SVG-format scalable graphics. In contrast to raster
 * formats, scalable vector graphics provide users and artists a way
 * to create, view, and provide imagery that is not limited to the
 * pixel or dot density that an output device is capable of.
 *
 * Many software developers use the libhisvg library to render
 * SVG graphics. It is lightweight and portable.
 */


#include "hisvg-private.h"
#include "hisvg-defs.h"
#include "hisvg-common.h"

extern double hisvg_internal_dpi_x;
extern double hisvg_internal_dpi_y;

G_DEFINE_TYPE_WITH_PRIVATE (HiSVGHandle, hisvg_handle, G_TYPE_OBJECT)

static void
hisvg_handle_init (HiSVGHandle * self)
{
    self->priv = hisvg_handle_get_instance_private (self);

    self->priv->flags = HISVG_HANDLE_FLAGS_NONE;
    self->priv->load_policy = HISVG_LOAD_POLICY_DEFAULT;
    self->priv->defs = hisvg_defs_new (self);
    self->priv->handler_nest = 0;
    self->priv->entities = g_hash_table_new_full (g_str_hash, 
                                                  g_str_equal,
                                                  g_free,
                                                  (GDestroyNotify) xmlFreeNode);
    self->priv->dpi_x = hisvg_internal_dpi_x;
    self->priv->dpi_y = hisvg_internal_dpi_y;

    self->priv->css_props = g_hash_table_new_full (g_str_hash,
                                                   g_str_equal,
                                                   g_free,
                                                   (GDestroyNotify) g_hash_table_destroy);

    self->priv->ctxt = NULL;
    self->priv->currentnode = NULL;
    self->priv->treebase = NULL;

    self->priv->finished = 0;
    self->priv->data_input_stream = NULL;
    self->priv->first_write = TRUE;
    self->priv->cancellable = NULL;

    self->priv->is_disposed = FALSE;
    self->priv->in_loop = FALSE;
    self->priv->inner_class_name_idx = 0;

    self->priv->css_buff = NULL;
    self->priv->css_buff_len = 0;
}

static void
hisvg_handle_dispose (GObject *instance)
{
    HiSVGHandle *self = (HiSVGHandle *) instance;

    if (self->priv->is_disposed)
      goto chain;

    self->priv->is_disposed = TRUE;

    g_hash_table_destroy (self->priv->entities);
    hisvg_defs_free (self->priv->defs);
    g_hash_table_destroy (self->priv->css_props);

    if (self->priv->title)
        g_string_free (self->priv->title, TRUE);
    if (self->priv->desc)
        g_string_free (self->priv->desc, TRUE);
    if (self->priv->metadata)
        g_string_free (self->priv->metadata, TRUE);
    if (self->priv->base_uri)
        g_free (self->priv->base_uri);

    if (self->priv->base_gfile) {
        g_object_unref (self->priv->base_gfile);
        self->priv->base_gfile = NULL;
    }
    if (self->priv->data_input_stream) {
        g_object_unref (self->priv->data_input_stream);
        self->priv->data_input_stream = NULL;
    }

    g_clear_object (&self->priv->cancellable);

    free(self->priv->css_buff);

  chain:
    G_OBJECT_CLASS (hisvg_handle_parent_class)->dispose (instance);
}


static void
hisvg_handle_class_init (HiSVGHandleClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = hisvg_handle_dispose;

    xmlInitParser ();

    hisvg_SAX_handler_struct_init ();
}

/**
 * hisvg_handle_destroy:
 * @handle: An #HiSVGHandle
 *
 * Frees @handle.
 **/
void hisvg_handle_destroy (HiSVGHandle * handle)
{
    g_object_unref (handle);
}

/**
 * hisvg_handle_new:
 *
 * Returns a new hisvg handle.  Must be freed with @g_object_unref.  This
 * handle can be used for dynamically loading an image.  You need to feed it
 * data using @hisvg_handle_write, then call @hisvg_handle_close when done.
 * Afterwords, you can render it using Cairo or get a GdkPixbuf from it. When
 * finished, free with g_object_unref(). No more than one image can be loaded 
 * with one handle.
 *
 * Returns: A new #HiSVGHandle
 **/
HiSVGHandle *
hisvg_handle_new (HiSVGHandleFlags flags)
{
    HiSVGHandle* handle = g_object_new (HISVG_TYPE_HANDLE, NULL);
    handle->priv->flags = flags;
    return handle;
}
