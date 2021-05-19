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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hisvg.h"
#include "hisvg-common.h"

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <unistd.h>


int MiniGUIMain (int argc, const char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: svg2png name.svg\n");
        return -1;
    }

    char* png_name = (char*)malloc(strlen(argv[1]) + 5);
    strcpy(png_name, argv[1]);
    strcat(png_name, ".png");
    fprintf(stderr, "in=%s|out=%s\n", argv[1], png_name);

    HiSVGDimensionData dimensions;
    GError *error = NULL;

#if 0
    char file_buf[1024] = {0};
    FILE* fp = fopen(argv[1], "r");
    fread(file_buf, 1024, 1, fp);
    fclose(fp);
    HiSVGHandle* svg = hisvg_handle_new_from_data (file_buf, strlen(file_buf), &error);
#else
    HiSVGHandle* svg = hisvg_handle_new_from_file (argv[1], &error);
#endif

    char id[100] = "#pp";
    HLDomElementNode* element = hisvg_handle_get_node(svg, id);
    fprintf(stderr, ".................find id=%s|element=%p\n", id, element);
    strcpy(id, "pp");
    element = hisvg_handle_get_node(svg, id);
    fprintf(stderr, ".................find id=%s|element=%p\n", id, element);

    char* css = "svg { color:blue !important; }";

    hisvg_handle_set_stylesheet (svg, NULL, css, strlen(css), NULL);

    hisvg_handle_get_dimensions_x (svg, &dimensions);
    fprintf(stderr, "dimensions w=%d|h=%d\n", dimensions.width, dimensions.height);

    HiSVGDimension hidim;
    hisvg_handle_get_dimensions (svg, &hidim);
    fprintf(stderr, "dimensions has_w=%d|has_h=%d|has_vbox=%d\n", hidim.has_w, hidim.has_h, hidim.has_vbox);
    fprintf(stderr, "dimensions w=%f|h=%f|vbox x=%f|y=%f|w=%f|h=%f\n", hidim.w.length, hidim.h.length, hidim.vbox.x, hidim.vbox.y, hidim.vbox.width, hidim.vbox.height);
    fprintf(stderr, "dimensions factor w=%c|h=%c\n", hidim.w.factor, hidim.h.factor);

//    cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, dimensions.width, dimensions.height);
    cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 200, 200);
    cairo_t* cr = cairo_create (surface);

#if 0
    cairo_set_source_rgb (cr,  1, 1, 1);
    cairo_paint (cr);
#endif

    HiSVGRect rect = {0, 0, 200, 200};
    hisvg_handle_render_cairo (svg, cr, &rect,  NULL, NULL);
//    hisvg_handle_render_cairo (svg, cr, NULL,  NULL, NULL);
    cairo_surface_write_to_png (surface, png_name);

#if 1
    while(1)
    {
        sleep(5);
    }
#endif

    cairo_surface_destroy (surface);
    cairo_destroy (cr);
    free(png_name);

    hisvg_handle_destroy(svg);

    return 0;
}
