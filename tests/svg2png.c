
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hisvg.h"
#include "hisvg-common.h"


int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: svg2png name.svg\n");
        return -1;
    }

    char* png_name = (char*)malloc(strlen(argv[1] + 5));
    strcpy(png_name, argv[1]);
    strcat(png_name, ".png");
    fprintf(stderr, "in=%s|out=%s\n", argv[1], png_name);

    HiSVGDimensionData dimensions;
    GError *error = NULL;
    HiSVGHandle* svg = hisvg_handle_new_from_file (argv[1], &error);

    char* id="#pp";
    HLDomElementNode* element = hisvg_handle_get_node(svg, id);
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
    cairo_surface_t* surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 100, 100);
    cairo_t* cr = cairo_create (surface);

#if 0
    cairo_set_source_rgb (cr,  1, 1, 1);
    cairo_paint (cr);
#endif

    HiSVGRect rect = {0, 0, 100, 100};
    hisvg_handle_render_cairo (svg, cr, &rect,  NULL, NULL);
    cairo_surface_write_to_png (surface, png_name);

    cairo_surface_destroy (surface);
    cairo_destroy (cr);
    free(png_name);

    hisvg_handle_destroy(svg);
    return 0;
}
