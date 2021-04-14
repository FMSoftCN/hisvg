
#include "hisvg.h"
#include "hisvg-base.h"
#include <glib.h>

void _hi_svg_set_load_flags(HiSVGLoadFlags* load_flags, HiSVGHandleFlags flags)
{
    if (load_flags == NULL)
    {
        return;
    }
    load_flags->unlimited_size = flags & HISVG_HANDLE_FLAG_UNLIMITED;
    load_flags->keep_image_data = flags & HISVG_HANDLE_FLAG_KEEP_IMAGE_DATA;
}

HiSVGHandleFlags _hi_svg_get_load_flags(HiSVGLoadFlags* load_flags)
{
    if (load_flags == NULL)
    {
        return HISVG_HANDLE_FLAGS_NONE;
    }

    return (load_flags->unlimited_size << 0) | (load_flags->keep_image_data << 1);
}

void _hi_svg_set_testing(HiSVGHandle* handle, uint8_t testing)
{
    if (handle)
    {
        handle->is_testing = testing;
    }
}

uint8_t _hi_svg_get_testing(HiSVGHandle* handle)
{
    return handle ? handle->is_testing : 0;
}

gboolean  _hisvg_handle_write (HiSVGHandle* handle, const guchar* buf,
                                     gsize count, GError** error)
{
    return FALSE;
}

gboolean  _hisvg_handle_close (HiSVGHandle* handle, GError** error)
{
    return FALSE;
}

gboolean hisvg_handle_fill_with_data (HiSVGHandle* handle, const char* data, 
        gsize data_len, GError** error)
{
    gboolean rv = FALSE;
    if (handle == NULL || data == NULL || data_len == 0)
    {
        return rv;
    }

    rv = _hisvg_handle_write (handle, (guchar *) data, data_len, error);
    return _hisvg_handle_close (handle, rv ? error : NULL) && rv;
}
