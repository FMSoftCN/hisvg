

#ifndef _HI_SVG_H_
#define _HI_SVG_H_

#include <glib.h>
#include "hidomlayout.h"

typedef struct HiSVGHandle_ HiSVGHandle;


typedef enum HiSVGHandleFlags_
{
    // default none
    HISVG_HANDLE_FLAGS_NONE           = 0,
    // Allow any SVG XML without size limitations.
    HISVG_HANDLE_FLAG_UNLIMITED       = 1 << 0,
    // Keeps the image data when for use by cairo.
    HISVG_HANDLE_FLAG_KEEP_IMAGE_DATA = 1 << 1
} HiSVGHandleFlags;

typedef struct HiSVGLength_ {
    double length;
    char factor;
} HiSVGLength;

typedef struct HiSVGRect_ {
    double x;
    double y;
    double width;
    double height;
} HiSVGRect;


typedef struct HiSVGDimension_ {
    HiSVGLength* width;
    HiSVGLength* height;
    HiSVGRect* viewbox;
} HiSVGDimension;

#ifdef __cplusplus
extern "C" {
#endif


HiSVGHandle* hisvg_handle_new (HiSVGHandleFlags flags);
void hisvg_handle_destroy (HiSVGHandle* handle);
void hisvg_handle_set_dpi (HiSVGHandle* handle, double dpi_x, double dpi_y);
void hisvg_handle_set_base_uri (HiSVGHandle* handle, const char* base_uri);
const char* hisvg_handle_get_base_uri (HiSVGHandle* handle);
gboolean hisvg_handle_has_sub (HiSVGHandle* handle, const char* id);
HiSVGHandle* hisvg_handle_new_from_data (const guint8* data, gsize data_len, GError** error);
HiSVGHandle* hisvg_handle_new_from_file (const gchar* file_name, GError** error);
gboolean hisvg_handle_set_stylesheet (HiSVGHandle* handle, const char* id, const guint8* css, gsize css_len, GError** error);
void hisvg_handle_get_dimensions (HiSVGHandle* handle, HiSVGDimension** dimension);
HLDomElementNode* hisvg_handle_get_node (HiSVGHandle* handle, const char* id);

#if 0
gboolean hisvg_handle_render_cairo (HiSVGHandle* handle, cairo_t* cr, const HiSVGRect* viewport, const char* id, GError** error);
gboolean hisvg_handle_get_geometry_for_layer (HiSVGHandle* handle,
                                    const char* id,
                                    const HiSVGRect* viewport,
                                    HiSVGRect* out_ink_rect,
                                    HiSVGRect* out_logical_rect,
                                    GError** error);

gboolean hisvg_handle_render_layer (HiSVGHandle*handle,
                          cairo_t* cr,
                          const char* id,
                          const HiSVGRect* viewport,
                          GError** error);

gboolean hisvg_handle_get_geometry_for_element (HiSVGHandle* handle,
                                      const char* id,
                                      HiSVGRect* out_ink_rect,
                                      HiSVGRect* out_logical_rect,
                                      GError** error);

gboolean hisvg_handle_render_element (HiSVGHandle* handle,
                            cairo_t* cr,
                            const char* id,
                            const HiSVGRect* element_viewport,
                            GError** error);

#endif

#ifdef __cplusplus
}
#endif

#endif // _HI_SVG_H_
