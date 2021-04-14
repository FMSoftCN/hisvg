#ifndef _HI_SVG_BASE_H_
#define _HI_SVG_BASE_H_

#include <libxml/SAX.h>

typedef struct HiSVGDpi_ {
    double x;
    double y;
} HiSVGDpi;

typedef struct HiSVGLoadFlags_ {
    uint8_t unlimited_size;
    uint8_t keep_image_data;
} HiSVGLoadFlags;

typedef struct HiSVGHandle_ {
    HiSVGDpi dpi;
    HiSVGLoadFlags load_flags;

    char* base_url;
    uint8_t is_testing;

    xmlParserCtxtPtr ctxt;
} HiSVGHandle;

#ifdef __cplusplus
extern "C" {
#endif

void _hi_svg_set_load_flags(HiSVGLoadFlags* load_flags, HiSVGHandleFlags flags);
HiSVGHandleFlags _hi_svg_get_load_flags(HiSVGLoadFlags* load_flags);

void _hi_svg_set_testing(HiSVGHandle* handle, uint8_t testing);
uint8_t _hi_svg_get_testing(HiSVGHandle* handle);

gboolean  _hisvg_handle_write (HiSVGHandle* handle, const guchar* buf,
                                     gsize count, GError** error);
gboolean  _hisvg_handle_close (HiSVGHandle* handle, GError** error);

gboolean _hisvg_handle_fill_with_data (HiSVGHandle* handle, const char* data, 
        gsize data_len, GError ** error);



#ifdef __cplusplus
}
#endif

#endif // _HI_SVG_BASE_H_

