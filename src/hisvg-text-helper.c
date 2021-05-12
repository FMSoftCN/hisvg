/////////////////////////////////////////////////////////////////////////////// //
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/**
 \verbatim

    This file is part of hiSVG. hiSVG is a  high performance SVG
    rendering library.

    Copyright (C) 2021 Beijing FMSoft Technologies Co., Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Or,

    As this program is a library, any link to this program must follow
    GNU General Public License version 3 (GPLv3). If you cannot accept
    GPLv3, you need to be licensed from FMSoft.

    If you have got a commercial license of this program, please use it
    under the terms and conditions of the commercial license.

    For more information about the commercial license, please refer to
    <http://www.minigui.com/blog/minigui-licensing-policy/>.

 \endverbatim
 */

#include <glib.h>
#include <cairo-ft.h>
#include "hisvg-text-helper.h"

#define HISVG_DEFAULT_FONT_TYPE "ttf"
#define HISVG_DEFAULT_FONT_FAMILY "serif"

HiSVGTextContext* hisvg_text_context_create (double dpi, const char* language, HiSVGTextDirection* direction, HiSVGTextGravity* gravity)
{
    HiSVGTextContext* ctx = (HiSVGTextContext*) calloc(1, sizeof(HiSVGTextContext));

    ctx->dpi = dpi;
    ctx->lang_code = language && strlen(language) >= 2 ? LanguageCodeFromISO639s1Code(language) : LANGCODE_unknown;
    if (direction)
    {
        switch (*direction)
        {
            case HISVG_TEXT_DIRECTION_LTR:
                ctx->base_dir = BIDI_PGDIR_LTR;
                break;
            case HISVG_TEXT_DIRECTION_RTL:
                ctx->base_dir = BIDI_PGDIR_RTL;
                break;
            case HISVG_TEXT_DIRECTION_TTB_LTR:   // FIXME
                ctx->base_dir = BIDI_PGDIR_LTR;
                break;
            case HISVG_TEXT_DIRECTION_TTB_RTL:   // FIXME
                ctx->base_dir = BIDI_PGDIR_RTL;
                break;
            case HISVG_TEXT_DIRECTION_WEAK_LTR:
                ctx->base_dir = BIDI_PGDIR_WLTR;
                break;
            case HISVG_TEXT_DIRECTION_WEAK_RTL:
                ctx->base_dir = BIDI_PGDIR_WRTL;
                break;
            case HISVG_TEXT_DIRECTION_NEUTRAL:
                ctx->base_dir = BIDI_PGDIR_ON;
                break;
        }
    }

    if (gravity)
    {
        switch (*gravity)
        {
            case HISVG_TEXT_GRAVITY_SOUTH:
                ctx->gravity = GLYPH_GRAVITY_SOUTH;
                break;
            case HISVG_TEXT_GRAVITY_EAST:
                ctx->gravity = GLYPH_GRAVITY_EAST;
                break;
            case HISVG_TEXT_GRAVITY_NORTH:
                ctx->gravity = GLYPH_GRAVITY_NORTH;
                break;
            case HISVG_TEXT_GRAVITY_WEST:
                ctx->gravity = GLYPH_GRAVITY_WEST;
                break;
            case HISVG_TEXT_GRAVITY_AUTO:
                ctx->gravity = GLYPH_GRAVITY_AUTO;
                break;
            default:
                ctx->gravity = GLYPH_GRAVITY_AUTO;
                break;
        }
    }
    else
    {
        ctx->gravity = GLYPH_GRAVITY_AUTO;
    }

    return ctx;
}

void hisvg_text_context_destroy (HiSVGTextContext* context)
{
    free(context);
}

HiSVGTextGravity hisvg_text_context_get_gravity (HiSVGTextContext* context)
{
    if (!context)
        return 0;

    switch (context->gravity)
    {
        case GLYPH_GRAVITY_SOUTH:
            return HISVG_TEXT_GRAVITY_SOUTH;
        case GLYPH_GRAVITY_EAST:
            return HISVG_TEXT_GRAVITY_EAST;
        case GLYPH_GRAVITY_NORTH:
            return HISVG_TEXT_GRAVITY_NORTH;
        case GLYPH_GRAVITY_WEST:
            return HISVG_TEXT_GRAVITY_WEST;
        case GLYPH_GRAVITY_AUTO:
            return HISVG_TEXT_GRAVITY_AUTO;
        default:
            return HISVG_TEXT_GRAVITY_AUTO;
    }
}

HiSVGTextContextLayout* hisvg_text_context_layout_create (HiSVGTextContext* context,
        int letter_spacing, HiSVGTextAlignment alignment, const HiSVGFontDescription* desc,
        int font_decor, uint32_t writing_mode, const char* text)
{
    PLOGFONT lf = NULL;
    if (text == NULL || strlen(text) == 0)
    {
        return NULL;
    }

    if (!(lf = CreateLogFontForMChar2UChar("utf-8")))
    {
        fprintf(stderr, "failed to create logfont for utf-8 charset\n");
        return NULL;
    }

    Uchar32* ucs;
    int consumed;
    int n;
    size_t left_len_text = strlen(text);
    consumed  = GetUCharsUntilParagraphBoundary(lf, text, left_len_text, WSR_NOWRAP, &ucs, &n);
    if (consumed <= 0)
    {
        DestroyLogFont(lf);
        return NULL;
    }

    int max_line_extent = 100;
    TEXTRUNS* tr = CreateTextRuns( ucs, n, context->lang_code, context->base_dir,
        desc->log_font, MakeRGB(0, 0, 0), 0, NULL);

    if (!InitComplexShapingEngine(tr)) {
        fprintf(stderr, "%s: InitComplexShapingEngine returns FALSE\n",
                __FUNCTION__);
        exit(1);
    }

    BreakOppo* bos = NULL;

    int bos_len = UStrGetBreaks(context->lang_code,
            CTR_CAPITALIZE, WBR_NORMAL, LBP_NORMAL,
            ucs, n, &bos);
    if (bos_len <= 0) {
        fprintf(stderr, "%s: UStrGetBreaks failed\n", __FUNCTION__);
        return NULL;
    }
    LAYOUT* layout = CreateLayout(tr, context->gravity | writing_mode,
        bos, TRUE, max_line_extent, 0, letter_spacing, letter_spacing, 4, NULL, 0);

    int w = 0;
    int h = 0;
    LAYOUTLINE* line = NULL;
    int32_t baseline = -1;
    while ((line = LayoutNextLine(layout, line, 0, FALSE, NULL, 0)))
    {
        SIZE size;
        GetLayoutLineSize(line, &size);
        if (baseline == -1)
        {
            baseline = size.cy;
        }
        size.cx += 5;
        size.cy += 5;

        switch (writing_mode) {
            case GRF_WRITING_MODE_HORIZONTAL_TB:
            case GRF_WRITING_MODE_HORIZONTAL_BT:
                w = w < size.cx ? size.cx : w;
                h += size.cy + 10;
                break;

            case GRF_WRITING_MODE_VERTICAL_RL:
            case GRF_WRITING_MODE_VERTICAL_LR:
                w += size.cx + 10;
                h = h < size.cx ? size.cx : h;
                break;
        }
    }

    HiSVGTextContextLayout* result = (HiSVGTextContextLayout*)calloc(1, sizeof(HiSVGTextContextLayout));
    result->context = context;
    result->layout = layout;
    result->tr = tr;
    result->bos = bos;
    result->ucs = ucs;
    result->writing_mode = writing_mode;
    result->lf = lf;

    result->font_size = desc->font_size;

    result->rect = (HiSVGTextRectangle*) calloc(1, sizeof(HiSVGTextRectangle));
    result->rect->x = 0;
    result->rect->y = 0;
    result->rect->width = w;
    result->rect->height = h;

    result->baseline = baseline;
    return result;
}

void hisvg_text_context_layout_destroy(HiSVGTextContextLayout* layout)
{
    free(layout->rect);
    DestroyTextRuns(layout->tr);
    DestroyLayout(layout->layout);
    free(layout->bos);
    free(layout->ucs);
    DestroyLogFont(layout->lf);
    free(layout);
}

void hisvg_text_context_layout_get_size (HiSVGTextContextLayout* layout, int* width, int* height)
{
    if (layout == NULL || layout->layout == NULL)
    {
        return;
    }

    if (width)
    {
        *width = layout->rect->width;
    }

    if (height)
    {
        *height = layout->rect->height;
    }
}

int hisvg_text_context_layout_get_baseline (HiSVGTextContextLayout* layout)
{
    if (layout == NULL || layout->layout == NULL)
    {
        return 0;
    }

    return layout->baseline;
}

HiSVGFontDescription* hisvg_font_description_create (const char* type,
        const char* family, HiSVGTextStyle style, HiSVGTextVariant variant,
        HiSVGTextWeight weight, HiSVGTextStretch stretch, int font_decoration,
        double size)
{
    char log_font_style[7] = {0};
    HiSVGFontDescription* desc = (HiSVGFontDescription*) calloc(1, sizeof(HiSVGFontDescription));
    desc->variant = variant;
    desc->stretch = stretch;
    desc->font_size = size;

    switch (weight)
    {
        case HISVG_TEXT_WEIGHT_THIN:
            log_font_style[0] = FONT_WEIGHT_THIN;
            break;
        case HISVG_TEXT_WEIGHT_ULTRALIGHT:
            log_font_style[0] = FONT_WEIGHT_EXTRA_LIGHT;
            break;
        case HISVG_TEXT_WEIGHT_LIGHT:
        case HISVG_TEXT_WEIGHT_SEMILIGHT:
        case HISVG_TEXT_WEIGHT_BOOK:
            log_font_style[0] = FONT_WEIGHT_LIGHT;
            break;
        case HISVG_TEXT_WEIGHT_NORMAL:
            log_font_style[0] = FONT_WEIGHT_NORMAL;
            break;
        case HISVG_TEXT_WEIGHT_MEDIUM:
            log_font_style[0] = FONT_WEIGHT_MEDIUM;
            break;
        case HISVG_TEXT_WEIGHT_SEMIBOLD:
            log_font_style[0] = FONT_WEIGHT_DEMIBOLD;
            break;
        case HISVG_TEXT_WEIGHT_BOLD:
            log_font_style[0] = FONT_WEIGHT_BOLD;
            break;
        case HISVG_TEXT_WEIGHT_ULTRABOLD:
            log_font_style[0] = FONT_WEIGHT_EXTRA_BOLD;
            break;
        case HISVG_TEXT_WEIGHT_HEAVY:
        case HISVG_TEXT_WEIGHT_ULTRAHEAVY:
            log_font_style[0] = FONT_WEIGHT_BLACK;
            break;
        default:
            log_font_style[0] = FONT_WEIGHT_BLACK;
            break;
    }

    switch(style)
    {
        case HISVG_TEXT_STYLE_NORMAL:
            log_font_style[1] = FONT_SLANT_ROMAN;
            break;
        case HISVG_TEXT_STYLE_OBLIQUE:
            log_font_style[1] = FONT_SLANT_OBLIQUE;
            break;
        case HISVG_TEXT_STYLE_ITALIC:
            log_font_style[1] = FONT_SLANT_ITALIC;
            break;
        default:
            log_font_style[1] = FONT_SLANT_ROMAN;
            break;
    }

    log_font_style[2] = FONT_FLIP_NONE;
    log_font_style[3] = FONT_OTHER_TTFNOCACHE;

    uint8_t has_underline = (font_decoration & TEXT_UNDERLINE);
    uint8_t has_strike = (font_decoration & TEXT_STRIKE);
    if (has_underline && has_strike)
    {
        log_font_style[4] = FONT_DECORATE_US;
    }
    else if (has_underline)
    {
        log_font_style[4] = FONT_DECORATE_UNDERLINE;
    }
    else if (has_strike)
    {
        log_font_style[4] = FONT_DECORATE_STRUCKOUT;
    }
    else
    {
        log_font_style[4] = FONT_DECORATE_NONE;
    }
    log_font_style[5] = FONT_RENDER_SUBPIXEL;
    log_font_style[6] = 0;

    // <fonttype>-<family[,aliase]*>-<styles>-<width>-<height>-<charset[,charset]*>
    snprintf(desc->log_font, 255, "%s-%s-%s-%d-%d-UTF-8",
            type ? type : HISVG_DEFAULT_FONT_TYPE,
            family ? family : HISVG_DEFAULT_FONT_FAMILY,
            log_font_style,
            (int)size,
            (int)size);
    return desc;
}

void hisvg_font_description_destroy (HiSVGFontDescription* desc)
{
    free(desc);
}

HiSVGTextContext* hisvg_text_layout_get_context (HiSVGTextContextLayout* layout)
{
    return layout->context;
}

void hisvg_text_context_layout_get_rect (HiSVGTextContextLayout* layout, HiSVGTextRectangle* rect)
{
    if (rect && layout->rect)
    {
        rect->x = layout->rect->x;
        rect->y = layout->rect->y;
        rect->width = layout->rect->width;
        rect->height = layout->rect->height;
    }
}

double hisvg_text_gravity_to_rotation (HiSVGTextGravity gravity)
{
    double rotation;
    switch (gravity)
    {
        default:
        case HISVG_TEXT_GRAVITY_AUTO: /* shut gcc up */
        case HISVG_TEXT_GRAVITY_SOUTH: rotation =  0;      break;
        case HISVG_TEXT_GRAVITY_NORTH: rotation =  G_PI;   break;
        case HISVG_TEXT_GRAVITY_EAST:  rotation = -G_PI_2; break;
        case HISVG_TEXT_GRAVITY_WEST:  rotation = +G_PI_2; break;
    }

    return rotation;
}

void hisvg_cairo_update_text_context (cairo_t* cr, HiSVGTextContext* context)
{
    context->cr = cr;
}

typedef struct _HiSVGLayoutParam {
    double x;
    double y;
    double font_size;
    int32_t baseline;
    cairo_t* cr;
    uint32_t writing_mode;
    void (*render) (cairo_t *cr, const cairo_glyph_t *glyphs, int num_glyphs);
}  HiSVGLayoutParam;


BOOL show_layout_cb (GHANDLE ctxt, Glyph32 glyph_value, const GLYPHPOS* glyph_pos, const RENDERDATA* render_data)
{
    HiSVGLayoutParam* param = (HiSVGLayoutParam*) ctxt;
    double space_size = param->font_size / 3;
    cairo_t* cr = param->cr;

    GLYPHINFO info = {0};
    info.mask = GLYPH_INFO_FACE | GLYPH_INFO_METRICS;
    cairo_glyph_t glyph;
    int ret = GetGlyphInfo (render_data->logfont, glyph_value, &info);
    if (ret != -1)
    {
        glyph.index = info.index;
        glyph.x = param->x;
        glyph.y = param->y + param->baseline;

        FT_Face ft_face = (FT_Face) info.ft_face;
        cairo_font_face_t* cairo_font_face = cairo_ft_font_face_create_for_ft_face (ft_face, 0);
        cairo_set_font_face(cr, cairo_font_face);
        cairo_set_font_size(cr, param->font_size);
        param->render(cr, &glyph, 1);
        cairo_font_face_destroy(cairo_font_face);

        switch (param->writing_mode) {
            case GRF_WRITING_MODE_HORIZONTAL_TB:
            case GRF_WRITING_MODE_HORIZONTAL_BT:
                param->x += info.advance_x;
                break;

            case GRF_WRITING_MODE_VERTICAL_RL:
            case GRF_WRITING_MODE_VERTICAL_LR:
                param->y += info.advance_y;
                break;
        }
    }
    else
    {
        switch (param->writing_mode) {
            case GRF_WRITING_MODE_HORIZONTAL_TB:
            case GRF_WRITING_MODE_HORIZONTAL_BT:
                param->x += space_size;
                break;

            case GRF_WRITING_MODE_VERTICAL_RL:
            case GRF_WRITING_MODE_VERTICAL_LR:
                param->y += space_size;
                break;
        }
    }
    return TRUE;
}

void _hisvg_cairo_render_layout(cairo_t* cr, HiSVGTextContextLayout* layout, HiSVGLayoutParam* param)
{
    int x = 0;
    int y = 0;
    LAYOUTLINE* line = NULL;
    while ((line = LayoutNextLine(layout->layout, line, 0, FALSE, show_layout_cb, param)))
    {
        RECT rc;
        GetLayoutLineRect(line, &x, &y, 0, &rc);

        switch (param->writing_mode) {
        case GRF_WRITING_MODE_HORIZONTAL_TB:
            y += 10;
            break;

        case GRF_WRITING_MODE_HORIZONTAL_BT:
            y -= 10;
            break;

        case GRF_WRITING_MODE_VERTICAL_RL:
            x -= 10;
            break;

        case GRF_WRITING_MODE_VERTICAL_LR:
            x += 10;
            break;
        }
        param->x = x;
        param->y = y;
    }
}


void hisvg_cairo_show_layout (cairo_t* cr, HiSVGTextContextLayout* layout)
{
    HiSVGLayoutParam param = {0};
    param.x = 0.0;
    param.y = 0.0;
    param.cr = cr;
    param.font_size = layout->font_size;
    param.baseline = layout->baseline;
    param.writing_mode = layout->writing_mode;
    param.render = cairo_show_glyphs;
    _hisvg_cairo_render_layout(cr, layout, &param);
}

void hisvg_cairo_layout_path (cairo_t* cr, HiSVGTextContextLayout* layout)
{
    HiSVGLayoutParam param = {0};
    param.x = 0.0;
    param.y = 0.0;
    param.cr = cr;
    param.font_size = layout->font_size;
    param.baseline = layout->baseline;
    param.writing_mode = layout->writing_mode;
    param.render = cairo_glyph_path;
    _hisvg_cairo_render_layout(cr, layout, &param);
}
