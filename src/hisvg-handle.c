
#include "hisvg.h"
#include "hisvg-base.h"
#include <glib.h>

HiSVGHandle* hisvg_handle_new (HiSVGHandleFlags flags)
{
    HiSVGHandle* handle = g_malloc0(sizeof(HiSVGHandle));
    if (handle == NULL)
    {
        return NULL;
    }
    handle->flags = flags;
    return handle;
}

void hisvg_handle_destroy (HiSVGHandle* handle)
{
    g_free(handle);
}
