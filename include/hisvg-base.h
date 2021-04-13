#ifndef _HI_SVG_PRIVATE_H_
#define _HI_SVG_PRIVATE_H_

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

    char* base_uri;

} HiSVGHandle;

#ifdef __cplusplus
extern "C" {
#endif

void _hi_svg_set_load_flags(HiSVGLoadFlags* load_flags, HiSVGHandleFlags flags);
HiSVGHandleFlags _hi_svg_get_load_flags(HiSVGLoadFlags* load_flags);

#ifdef __cplusplus
}
#endif

#endif // _HI_SVG_PRIVATE_H_

