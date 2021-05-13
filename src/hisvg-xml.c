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

    Copyright © 2010 Christian Persch
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

#include "hisvg-xml.h"

typedef struct {
    GInputStream *stream;
    GCancellable *cancellable;
    GError      **error;
} HiSVGXmlInputStreamContext;

/* this should use gsize, but libxml2 is borked */
static int
context_read (HiSVGXmlInputStreamContext *context,
              char *buffer,
              int   len)
{
    gssize n_read;

    if (*(context->error))
        return -1;

    n_read = g_input_stream_read (context->stream, buffer, (gsize) len,
                                  context->cancellable,
                                  context->error);
    if (n_read < 0)
        return -1;

    return (int) n_read;
}

static int
context_close (HiSVGXmlInputStreamContext *context)
{
    gboolean ret;

    /* Don't overwrite a previous error */
    ret = g_input_stream_close (context->stream, context->cancellable,
                                *(context->error) == NULL ? context->error : NULL);

    g_object_unref (context->stream);
    if (context->cancellable)
        g_object_unref (context->cancellable);
    g_slice_free (HiSVGXmlInputStreamContext, context);

    return ret ? 0 : -1;
}

/**
 * _hisvg_xml_input_buffer_new_from_stream:
 * @context: a #xmlParserCtxtPtr
 * @input_stream: a #GInputStream
 *
 * Returns: a new #xmlParserInputPtr wrapping @input_stream
 */
xmlParserInputBufferPtr
_hisvg_xml_input_buffer_new_from_stream (GInputStream   *stream,
                                        GCancellable   *cancellable,
                                        xmlCharEncoding enc,
                                        GError        **error)

{
    HiSVGXmlInputStreamContext *context;

    g_return_val_if_fail (G_IS_INPUT_STREAM (stream), NULL);
    g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), NULL);
    g_return_val_if_fail (error != NULL, NULL);

    context = g_slice_new (HiSVGXmlInputStreamContext);
    context->stream = g_object_ref (stream);
    context->cancellable = cancellable ? g_object_ref (cancellable) : NULL;
    context->error = error;

    return xmlParserInputBufferCreateIO ((xmlInputReadCallback) context_read,
                                         (xmlInputCloseCallback) context_close,
                                         context,
                                         enc);
}
