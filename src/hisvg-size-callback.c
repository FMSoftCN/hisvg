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


#include "hisvg-size-callback.h"

#include <math.h>

void
_hisvg_size_callback (int *width, int *height, gpointer data)
{
    struct HiSVGSizeCallbackData *real_data = (struct HiSVGSizeCallbackData *) data;
    double zoomx, zoomy, zoom;

    int in_width, in_height;

    in_width = *width;
    in_height = *height;

    switch (real_data->type) {
    case HISVG_SIZE_ZOOM:
        if (*width < 0 || *height < 0)
            return;

        *width = floor (real_data->x_zoom * *width + 0.5);
        *height = floor (real_data->y_zoom * *height + 0.5);
        break;

    case HISVG_SIZE_ZOOM_MAX:
        if (*width < 0 || *height < 0)
            return;

        *width = floor (real_data->x_zoom * *width + 0.5);
        *height = floor (real_data->y_zoom * *height + 0.5);

        if (*width > real_data->width || *height > real_data->height) {
            zoomx = (double) real_data->width / *width;
            zoomy = (double) real_data->height / *height;
            zoom = MIN (zoomx, zoomy);

            *width = floor (zoom * *width + 0.5);
            *height = floor (zoom * *height + 0.5);
        }
        break;

    case HISVG_SIZE_WH_MAX:
        if (*width < 0 || *height < 0)
            return;

        zoomx = (double) real_data->width / *width;
        zoomy = (double) real_data->height / *height;
        if (zoomx < 0)
            zoom = zoomy;
        else if (zoomy < 0)
            zoom = zoomx;
        else
            zoom = MIN (zoomx, zoomy);

        *width = floor (zoom * *width + 0.5);
        *height = floor (zoom * *height + 0.5);
        break;

    case HISVG_SIZE_WH:
        if (real_data->width != -1)
            *width = real_data->width;
        if (real_data->height != -1)
            *height = real_data->height;
        break;

    default:
        g_assert_not_reached ();
    }

    if (real_data->keep_aspect_ratio) {
        int out_min = MIN (*width, *height);

        if (out_min == *width) {
            *height = in_height * ((double) *width / (double) in_width);
        } else {
            *width = in_width * ((double) *height / (double) in_height);
        }
    }
}
