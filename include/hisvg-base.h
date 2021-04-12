#ifndef _HI_SVG_PRIVATE_H_
#define _HI_SVG_PRIVATE_H_

typedef struct HiSVGHandle_ {

    HiSVGHandleFlags flags;

    double dpi_x;
    double dpi_y;

    char* base_uri;

} HiSVGHandle;

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif // _HI_SVG_PRIVATE_H_

