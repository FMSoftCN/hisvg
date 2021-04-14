
#include "hisvg.h"
#include "hisvg-base.h"

#define HISVG_DEFAULT_DPI_X 90.0
#define HISVG_DEFAULT_DPI_Y 90.0

GQuark hisvg_error_quark (void)
{
    return g_quark_from_string ("hisvg-error-quark");
}

HiSVGHandle* hisvg_handle_new (HiSVGHandleFlags flags)
{
    HiSVGHandle* handle = g_malloc0(sizeof(HiSVGHandle));
    if (handle == NULL)
    {
        return NULL;
    }
    _hisvg_set_load_flags(&handle->load_flags, flags);
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

    if (handle->base_url)
    {
        g_free(handle->base_url);
    }
    handle->base_url = g_strdup(base_uri);
}

const char* hisvg_handle_get_base_uri (HiSVGHandle* handle)
{
    return handle ? handle->base_url : NULL;
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

