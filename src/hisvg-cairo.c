
#include "hisvg.h"
#include "hisvg-base.h"
#include <glib.h>


gboolean hisvg_handle_render_cairo (HiSVGHandle* handle, cairo_t* cr, const HiSVGRect* viewport, const char* id, GError** error)
{
    return FALSE;
}

gboolean hisvg_handle_get_geometry_for_layer (HiSVGHandle* handle,
                                    const char* id,
                                    const HiSVGRect* viewport,
                                    HiSVGRect* out_ink_rect,
                                    HiSVGRect* out_logical_rect,
                                    GError** error)
{
    return FALSE;
}

gboolean hisvg_handle_render_layer (HiSVGHandle*handle,
                          cairo_t* cr,
                          const char* id,
                          const HiSVGRect* viewport,
                          GError** error)
{
    return FALSE;
}

gboolean hisvg_handle_get_geometry_for_element (HiSVGHandle* handle,
                                      const char* id,
                                      HiSVGRect* out_ink_rect,
                                      HiSVGRect* out_logical_rect,
                                      GError** error)
{
    return FALSE;
}

gboolean hisvg_handle_render_element (HiSVGHandle* handle,
                            cairo_t* cr,
                            const char* id,
                            const HiSVGRect* element_viewport,
                            GError** error)
{
    return FALSE;
}
