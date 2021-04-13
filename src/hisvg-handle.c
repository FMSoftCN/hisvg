
#include "hisvg.h"
#include "hisvg-base.h"
#include <glib.h>

#define HISVG_DEFAULT_DPI_X 90.0
#define HISVG_DEFAULT_DPI_Y 90.0

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

HiSVGHandle* hisvg_handle_new (HiSVGHandleFlags flags)
{
    HiSVGHandle* handle = g_malloc0(sizeof(HiSVGHandle));
    if (handle == NULL)
    {
        return NULL;
    }
    _hi_svg_set_load_flags(&handle->load_flags, flags);
    return handle;
}

void hisvg_handle_destroy (HiSVGHandle* handle)
{
    g_free(handle);
}

void hisvg_handle_set_dpi (HiSVGHandle* handle, double dpi_x, double dpi_y)
{
    if (handle == NULL)
    {
        return;
    }

    if (dpi_x <= 0.)
        handle->dpi.x = HISVG_DEFAULT_DPI_X;
    else
        handle->dpi.x = dpi_x;

    if (dpi_y <= 0.)
        handle->dpi.y = HISVG_DEFAULT_DPI_Y;
    else
        handle->dpi.y = dpi_y;
}

void hisvg_handle_set_base_uri (HiSVGHandle* handle, const char* base_uri)
{
    if (handle == NULL)
    {
        return;
    }

    if (handle->base_uri)
    {
        g_free(handle->base_uri);
    }
    handle->base_uri = g_strdup(base_uri);
}

const char* hisvg_handle_get_base_uri (HiSVGHandle* handle)
{
    return handle ? handle->base_uri : NULL;
}

// TODO
gboolean hisvg_handle_has_sub (HiSVGHandle* handle, const char* id)
{
    return FALSE;
}

// TODO
HiSVGHandle* hisvg_handle_new_from_data (const guint8* data, gsize data_len, GError** error)
{
    return NULL;
}

// TODO
HiSVGHandle* hisvg_handle_new_from_file (const gchar* file_name, GError** error)
{
    return NULL;
}

// TODO
gboolean hisvg_handle_set_stylesheet (HiSVGHandle* handle, const char* id, const guint8* css, gsize css_len, GError** error)
{
    return FALSE;
}

// TODO
void hisvg_handle_get_dimensions (HiSVGHandle* handle, HiSVGDimension** dimension)
{
}

// TODO
HLDomElementNode* hisvg_handle_get_node (HiSVGHandle* handle, const char* id)
{
    return NULL;
}
