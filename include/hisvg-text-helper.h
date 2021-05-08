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

#ifndef _HI_SVG_TEXT_HELPER_H_
#define _HI_SVG_TEXT_HELPER_H_

#include <cairo.h>
#include <pango/pangocairo.h>

enum {
    TEXT_NORMAL = 0x00,
    TEXT_OVERLINE = 0x01,
    TEXT_UNDERLINE = 0x02,
    TEXT_STRIKE = 0x04
};

typedef enum {
    TEXT_ANCHOR_START,
    TEXT_ANCHOR_MIDDLE,
    TEXT_ANCHOR_END
} TextAnchor;

typedef enum {
    UNICODE_BIDI_NORMAL = 0,
    UNICODE_BIDI_EMBED = 1,
    UNICODE_BIDI_OVERRIDE = 2
} UnicodeBidi;

typedef enum {
    HISVG_ENABLE_BACKGROUND_ACCUMULATE,
    HISVG_ENABLE_BACKGROUND_NEW
} HiSVGEnableBackgroundType;


typedef enum {
  HISVG_TEXT_STYLE_NORMAL,
  HISVG_TEXT_STYLE_OBLIQUE,
  HISVG_TEXT_STYLE_ITALIC
} HiSVGTextStyle;

typedef enum {
  HISVG_TEXT_VARIANT_NORMAL,
  HISVG_TEXT_VARIANT_SMALL_CAPS
} HiSVGTextVariant;

typedef enum {
  HISVG_TEXT_WEIGHT_THIN = 100,
  HISVG_TEXT_WEIGHT_ULTRALIGHT = 200,
  HISVG_TEXT_WEIGHT_LIGHT = 300,
  HISVG_TEXT_WEIGHT_SEMILIGHT = 350,
  HISVG_TEXT_WEIGHT_BOOK = 380,
  HISVG_TEXT_WEIGHT_NORMAL = 400,
  HISVG_TEXT_WEIGHT_MEDIUM = 500,
  HISVG_TEXT_WEIGHT_SEMIBOLD = 600,
  HISVG_TEXT_WEIGHT_BOLD = 700,
  HISVG_TEXT_WEIGHT_ULTRABOLD = 800,
  HISVG_TEXT_WEIGHT_HEAVY = 900,
  HISVG_TEXT_WEIGHT_ULTRAHEAVY = 1000
} HiSVGTextWeight;


typedef enum {
  HISVG_TEXT_STRETCH_ULTRA_CONDENSED,
  HISVG_TEXT_STRETCH_EXTRA_CONDENSED,
  HISVG_TEXT_STRETCH_CONDENSED,
  HISVG_TEXT_STRETCH_SEMI_CONDENSED,
  HISVG_TEXT_STRETCH_NORMAL,
  HISVG_TEXT_STRETCH_SEMI_EXPANDED,
  HISVG_TEXT_STRETCH_EXPANDED,
  HISVG_TEXT_STRETCH_EXTRA_EXPANDED,
  HISVG_TEXT_STRETCH_ULTRA_EXPANDED
} HiSVGTextStretch;

typedef enum {
  HISVG_TEXT_DIRECTION_LTR,
  HISVG_TEXT_DIRECTION_RTL,
  HISVG_TEXT_DIRECTION_TTB_LTR,
  HISVG_TEXT_DIRECTION_TTB_RTL,
  HISVG_TEXT_DIRECTION_WEAK_LTR,
  HISVG_TEXT_DIRECTION_WEAK_RTL,
  HISVG_TEXT_DIRECTION_NEUTRAL
} HiSVGTextDirection;

typedef enum {
  HISVG_TEXT_GRAVITY_SOUTH,
  HISVG_TEXT_GRAVITY_EAST,
  HISVG_TEXT_GRAVITY_NORTH,
  HISVG_TEXT_GRAVITY_WEST,
  HISVG_TEXT_GRAVITY_AUTO
} HiSVGTextGravity;

typedef enum {
  HISVG_TEXT_UNDERLINE_NONE,
  HISVG_TEXT_UNDERLINE_SINGLE,
  HISVG_TEXT_UNDERLINE_DOUBLE,
  HISVG_TEXT_UNDERLINE_LOW,
  HISVG_TEXT_UNDERLINE_ERROR
} HiSVGTextUnderline;

typedef enum {
  HISVG_TEXT_ALIGN_LEFT,
  HISVG_TEXT_ALIGN_CENTER,
  HISVG_TEXT_ALIGN_RIGHT
} HiSVGTextAlignment;

typedef struct _PangoFontMap HiSVGFontMap;
typedef struct _PangoFontDescription HiSVGFontDescription;
typedef struct _PangoContext HiSVGTextContext;
typedef struct _PangoLayout HiSVGTextContextLayout;
typedef struct _PangoLayoutIter HiSVGTextContextLayoutIter;
typedef struct _PangoRectangle HiSVGTextRectangle;
typedef struct _PangoLanguage HiSVGTextLanguage;
typedef struct _PangoAttrList HiSVGTextAttrList;
typedef struct _PangoAttribute HiSVGTextAttribute;


HiSVGFontMap* hisvg_font_map_get_default ();
HiSVGTextContext* hisvg_create_text_context (HiSVGFontMap* fontmap);
void hisvg_text_context_set_resolution (HiSVGTextContext* context, double dpi);
HiSVGTextGravity hisvg_text_context_get_gravity (HiSVGTextContext* context);
void hisvg_text_context_set_language (HiSVGTextContext* context, HiSVGTextLanguage* language);
void hisvg_text_context_set_base_dir (HiSVGTextContext* context, HiSVGTextDirection direction);
void hisvg_text_context_set_base_gravity (HiSVGTextContext* context, HiSVGTextGravity gravity);

HiSVGTextContextLayout* hisvg_text_context_layout_new (HiSVGTextContext* context);
HiSVGTextContextLayoutIter* hisvg_text_context_layout_get_iter (HiSVGTextContextLayout* layout);
void hisvg_text_context_layout_get_size (HiSVGTextContextLayout* layout, int* width, int* height);
void hisvg_text_context_layout_iter_free (HiSVGTextContextLayoutIter* iter);
int hisvg_text_context_layout_iter_get_baseline (HiSVGTextContextLayoutIter* iter);
void hisvg_text_context_layout_set_alignment (HiSVGTextContextLayout* layout, HiSVGTextAlignment alignment);
void hisvg_text_context_layout_set_attributes (HiSVGTextContextLayout* layout, HiSVGTextAttrList* attrs); 
void hisvg_text_context_layout_set_text (HiSVGTextContextLayout* layout, const char* text, int length);
void hisvg_text_context_layout_set_font_description (HiSVGTextContextLayout* layout, const HiSVGFontDescription* desc);

HiSVGFontDescription* hisvg_text_context_get_font_description (HiSVGTextContext* context);
HiSVGFontDescription* hisvg_font_description_copy (const HiSVGFontDescription* desc);
void hisvg_font_description_set_family_static (HiSVGFontDescription* desc,  const char* family);
void hisvg_font_description_set_style (HiSVGFontDescription* desc, HiSVGTextStyle style);
void hisvg_font_description_set_variant (HiSVGFontDescription* desc, HiSVGTextVariant variant);
void hisvg_font_description_set_weight (HiSVGFontDescription* desc, HiSVGTextWeight weight);
void hisvg_font_description_set_stretch (HiSVGFontDescription* desc, HiSVGTextStretch stretch);
void hisvg_font_description_set_size (HiSVGFontDescription* desc, gint size);
void hisvg_font_description_free (HiSVGFontDescription* desc);

HiSVGTextAttrList*  hisvg_text_attr_list_new (void);
void hisvg_text_attr_list_unref (HiSVGTextAttrList* list);
void hisvg_text_attr_list_insert (HiSVGTextAttrList* list, PangoAttribute* attr);

HiSVGTextAttribute* hisvg_text_attr_strikethrough_new (gboolean strikethrough);
HiSVGTextAttribute* hisvg_text_attr_underline_new (PangoUnderline underline);
HiSVGTextAttribute* hisvg_text_attr_letter_spacing_new (int letter_spacing);

HiSVGTextContext* hisvg_text_layout_get_context (HiSVGTextContextLayout* layout);
void hisvg_text_context_layout_get_extents (HiSVGTextContextLayout* layout, HiSVGTextRectangle* ink_rect, HiSVGTextRectangle* logical_rect);
double hisvg_text_gravity_to_rotation (HiSVGTextGravity gravity);

HiSVGTextLanguage* hisvg_text_language_from_string (const char *language);

void hisvg_cairo_update_text_context (cairo_t* cr, HiSVGTextContext* context);
void hisvg_cairo_show_layout (cairo_t* cr, HiSVGTextContextLayout* layout);
void hisvg_cairo_layout_path (cairo_t* cr, HiSVGTextContextLayout* layout);


#endif // _HI_SVG_TEXT_HELPER_H_