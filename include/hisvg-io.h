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
    Copyright (C) 2002, 2003, 2004, 2005 Dom Lachowicz <cinamod@hotmail.com>
    Copyright (C) 2003, 2004, 2005 Caleb Moore <c.moore@student.unsw.edu.au>
    Copyright © 2011, 2012 Christian Persch
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

#ifndef HISVG_IO_H
#define HISVG_IO_H

#include <glib.h>
#include <gio/gio.h>

G_GNUC_INTERNAL
gchar *_hisvg_io_get_file_path (const gchar *filename, 
                               const gchar *basedir);

char *_hisvg_io_acquire_data (const char *uri,
			     const char *base_uri,
			     char **mime_type,
			     gsize *len,
			     GCancellable *cancellable,
			     GError **error);

GInputStream *_hisvg_io_acquire_stream (const char *uri,
                                       const char *base_uri,
                                       char **mime_type,
                                       GCancellable *cancellable,
                                       GError **error);

#endif /* HISVG_IO_H */
