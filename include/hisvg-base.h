#ifndef _HI_SVG_BASE_H_
#define _HI_SVG_BASE_H_

#include "hisvg.h"
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
    GError** error;

    gboolean is_closed;
    gboolean finished;
} HiSVGHandle;

#ifdef __cplusplus
extern "C" {
#endif

void _hisvg_set_load_flags(HiSVGLoadFlags* load_flags, HiSVGHandleFlags flags);
HiSVGHandleFlags _hisvg_get_load_flags(HiSVGLoadFlags* load_flags);

void _hisvg_set_testing(HiSVGHandle* handle, uint8_t testing);
uint8_t _hisvg_get_testing(HiSVGHandle* handle);

void _hisvg_return_if_fail_warning (const char *pretty_function,
                                  const char *expression, GError ** error);
void _hisvg_set_error (GError** error, xmlParserCtxtPtr ctxt);

gboolean  _hisvg_handle_write (HiSVGHandle* handle, const guchar* buf,
                                     gsize count, GError** error);
gboolean  _hisvg_handle_close (HiSVGHandle* handle, GError** error);

gboolean _hisvg_handle_fill_with_data (HiSVGHandle* handle, const char* data, 
        gsize data_len, GError ** error);



#ifdef __cplusplus
}
#endif

#define _hisvg_return_if_fail(expr, error)    G_STMT_START{			\
     if G_LIKELY(expr) { } else                                     \
       {                                                            \
           _hisvg_return_if_fail_warning (G_STRFUNC,                  \
                                        #expr, error);              \
           return;                                                  \
       };				}G_STMT_END

#define _hisvg_return_val_if_fail(expr,val,error)	G_STMT_START{       \
     if G_LIKELY(expr) { } else                                     \
       {                                                            \
           _hisvg_return_if_fail_warning (G_STRFUNC,                  \
                                        #expr, error);              \
           return (val);                                            \
       };				}G_STMT_END

#endif // _HI_SVG_BASE_H_

