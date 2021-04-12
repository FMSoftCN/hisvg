

#ifndef _HI_SVG_H_
#define _HI_SVG_H_

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

#ifdef __cplusplus
extern "C" {
#endif


HiSVGHandle* hisvg_handle_new (HiSVGHandleFlags flags);
void hisvg_handle_destroy (HiSVGHandle* handle);

#ifdef __cplusplus
}
#endif

#endif // _HI_SVG_H_
