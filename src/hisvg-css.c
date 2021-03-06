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

    Copyright (C) 2000 Eazel, Inc.
    Copyright (C) 2021 Beijing FMSoft Technologies Co., Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General License for more details.

    You should have received a copy of the GNU Lesser General License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Or,

    As this program is a library, any link to this program must follow
    GNU Lesser General License version 3 (LGPLv3). If you cannot accept
    LGPLv3, you need to be licensed from FMSoft.

    If you have got a commercial license of this program, please use it
    under the terms and conditions of the commercial license.

    For more information about the commercial license, please refer to
    <http://www.minigui.com/blog/minigui-licensing-policy/>.

 \endverbatim
 */

#include "hisvg-css.h"
#include "hisvg-private.h"
#include "hisvg-styles.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <libxml/parser.h>


#define POINTS_PER_INCH (72.0)
#define CM_PER_INCH     (2.54)
#define MM_PER_INCH     (25.4)
#define PICA_PER_INCH   (6.0)

#define SETINHERIT() G_STMT_START {if (inherit != NULL) *inherit = TRUE;} G_STMT_END
#define UNSETINHERIT() G_STMT_START {if (inherit != NULL) *inherit = FALSE;} G_STMT_END

/**
 * hisvg_css_parse_vbox:
 * @vbox: The CSS viewBox
 * @x : The X output
 * @y: The Y output
 * @w: The Width output
 * @h: The Height output
 *
 * Returns: 
 */
HiSVGViewBox
hisvg_css_parse_vbox (const char *vbox)
{
    HiSVGViewBox vb;
    gdouble *list;
    guint list_len;
    vb.active = FALSE;

    vb.rect.x = vb.rect.y = 0;
    vb.rect.width = vb.rect.height = 0;

    list = hisvg_css_parse_number_list (vbox, &list_len);

    if (!(list && list_len))
        return vb;
    else if (list_len != 4) {
        g_free (list);
        return vb;
    } else {
        vb.rect.x = list[0];
        vb.rect.y = list[1];
        vb.rect.width = list[2];
        vb.rect.height = list[3];
        vb.active = TRUE;

        g_free (list);
        return vb;
    }
}

typedef enum _RelativeSize {
    RELATIVE_SIZE_NORMAL,
    RELATIVE_SIZE_SMALLER,
    RELATIVE_SIZE_LARGER
} RelativeSize;

static double
hisvg_css_parse_raw_length (const char *str, gboolean * in,
                           gboolean * percent, gboolean * em, gboolean * ex, RelativeSize * relative_size)
{
    double length = 0.0;
    char *p = NULL;

    /* 
     *  The supported CSS length unit specifiers are: 
     *  em, ex, px, pt, pc, cm, mm, in, and %
     */
    *percent = FALSE;
    *em = FALSE;
    *ex = FALSE;
    *relative_size = RELATIVE_SIZE_NORMAL;

    length = g_ascii_strtod (str, &p);

    if ((length == -HUGE_VAL || length == HUGE_VAL) && (ERANGE == errno)) {
        /* todo: error condition - figure out how to best represent it */
        return 0.0;
    }

    /* test for either pixels or no unit, which is assumed to be pixels */
    if (p && *p && (strcmp (p, "px") != 0)) {
        if (!strcmp (p, "pt")) {
            length /= POINTS_PER_INCH;
            *in = TRUE;
        } else if (!strcmp (p, "in"))
            *in = TRUE;
        else if (!strcmp (p, "cm")) {
            length /= CM_PER_INCH;
            *in = TRUE;
        } else if (!strcmp (p, "mm")) {
            length /= MM_PER_INCH;
            *in = TRUE;
        } else if (!strcmp (p, "pc")) {
            length /= PICA_PER_INCH;
            *in = TRUE;
        } else if (!strcmp (p, "em"))
            *em = TRUE;
        else if (!strcmp (p, "ex"))
            *ex = TRUE;
        else if (!strcmp (p, "%")) {
            *percent = TRUE;
            length *= 0.01;
        } else {
            double pow_factor = 0.0;

            if (!g_ascii_strcasecmp (p, "larger")) {
                *relative_size = RELATIVE_SIZE_LARGER;
                return 0.0;
            } else if (!g_ascii_strcasecmp (p, "smaller")) {
                *relative_size = RELATIVE_SIZE_SMALLER;
                return 0.0;
            } else if (!g_ascii_strcasecmp (p, "xx-small")) {
                pow_factor = -3.0;
            } else if (!g_ascii_strcasecmp (p, "x-small")) {
                pow_factor = -2.0;
            } else if (!g_ascii_strcasecmp (p, "small")) {
                pow_factor = -1.0;
            } else if (!g_ascii_strcasecmp (p, "medium")) {
                pow_factor = 0.0;
            } else if (!g_ascii_strcasecmp (p, "large")) {
                pow_factor = 1.0;
            } else if (!g_ascii_strcasecmp (p, "x-large")) {
                pow_factor = 2.0;
            } else if (!g_ascii_strcasecmp (p, "xx-large")) {
                pow_factor = 3.0;
            } else {
                return 0.0;
            }

            length = 12.0 * pow (1.2, pow_factor) / POINTS_PER_INCH;
            *in = TRUE;
        }
    }

    return length;
}

HiSVGLength
_hisvg_css_parse_length (const char *str)
{
    HiSVGLength out;
    gboolean percent, em, ex, in;
    RelativeSize relative_size = RELATIVE_SIZE_NORMAL;
    percent = em = ex = in = FALSE;

    out.length = hisvg_css_parse_raw_length (str, &in, &percent, &em, &ex, &relative_size);
    if (percent)
        out.factor = 'p';
    else if (em)
        out.factor = 'm';
    else if (ex)
        out.factor = 'x';
    else if (in)
        out.factor = 'i';
    else if (relative_size == RELATIVE_SIZE_LARGER)
        out.factor = 'l';
    else if (relative_size == RELATIVE_SIZE_SMALLER)
        out.factor = 's';
    else
        out.factor = '\0';
    return out;
}

/* Recursive evaluation of all parent elements regarding absolute font size */
double
_hisvg_css_normalize_font_size (HiSVGState * state, HiSVGDrawingCtx * ctx)
{
    HiSVGState *parent;

    switch (state->font_size.factor) {
    case 'p':
    case 'm':
    case 'x':
        parent = hisvg_state_parent (state);
        if (parent) {
            double parent_size;
            parent_size = _hisvg_css_normalize_font_size (parent, ctx);
            return state->font_size.length * parent_size;
        }
        break;
    default:
        return _hisvg_css_normalize_length (&state->font_size, ctx, 'v');
        break;
    }

    return 12.;
}

double
_hisvg_css_normalize_length (const HiSVGLength * in, HiSVGDrawingCtx * ctx, char dir)
{
    if (in->factor == '\0')
        return in->length;
    else if (in->factor == 'p') {
        if (dir == 'h')
            return in->length * ctx->vb.rect.width;
        if (dir == 'v')
            return in->length * ctx->vb.rect.height;
        if (dir == 'o')
            return in->length * hisvg_viewport_percentage (ctx->vb.rect.width,
                                                          ctx->vb.rect.height);
    } else if (in->factor == 'm' || in->factor == 'x') {
        double font = _hisvg_css_normalize_font_size (hisvg_current_state (ctx), ctx);
        if (in->factor == 'm')
            return in->length * font;
        else
            return in->length * font / 2.;
    } else if (in->factor == 'i') {
        if (dir == 'h')
            return in->length * ctx->dpi_x;
        if (dir == 'v')
            return in->length * ctx->dpi_y;
        if (dir == 'o')
            return in->length * hisvg_viewport_percentage (ctx->dpi_x, ctx->dpi_y);
    } else if (in->factor == 'l') {
        /* todo: "larger" */
    } else if (in->factor == 's') {
        /* todo: "smaller" */
    }

    return 0;
}

/* Recursive evaluation of all parent elements regarding basline-shift */
double
_hisvg_css_accumulate_baseline_shift (HiSVGState * state, HiSVGDrawingCtx * ctx)
{
    HiSVGState *parent;
    double shift = 0.;

    parent = hisvg_state_parent (state);
    if (parent) {
        if (state->has_baseline_shift) {
            double parent_font_size;
            parent_font_size = _hisvg_css_normalize_font_size (parent, ctx); /* font size from here */
            shift = parent_font_size * state->baseline_shift;
        }
        shift += _hisvg_css_accumulate_baseline_shift (parent, ctx); /* baseline-shift for parent element */
    }

    return shift;
}


double
_hisvg_css_hand_normalize_length (const HiSVGLength * in, gdouble pixels_per_inch,
                                 gdouble width_or_height, gdouble font_size)
{
    if (in->factor == '\0')
        return in->length;
    else if (in->factor == 'p')
        return in->length * width_or_height;
    else if (in->factor == 'm')
        return in->length * font_size;
    else if (in->factor == 'x')
        return in->length * font_size / 2.;
    else if (in->factor == 'i')
        return in->length * pixels_per_inch;

    return 0;
}

static gint
hisvg_css_clip_rgb_percent (const char *s, double max)
{
    double value;
    char *end;

    value = g_ascii_strtod (s, &end);

    if (*end == '%') {
        value = CLAMP (value, 0, 100) / 100.0;
    }
    else {
        value = CLAMP (value, 0, max) / max;
    }
    
    return (gint) floor (value * 255 + 0.5);
}

/* pack 3 [0,255] ints into one 32 bit one */
#define PACK_RGBA(r,g,b,a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define PACK_RGB(r,g,b) PACK_RGBA(r, g, b, 255)

/**
 * hisvg_css_parse_color:
 * @str: string to parse
 * @inherit: whether to inherit
 *
 * Parse a CSS2 color specifier, return RGB value
 *
 * Returns: and RGB value
 */
guint32
hisvg_css_parse_color (const char *str, gboolean * inherit)
{
    gint val = 0;

    SETINHERIT ();

    if (str[0] == '#') {
        int i;
        for (i = 1; str[i]; i++) {
            int hexval;
            if (str[i] >= '0' && str[i] <= '9')
                hexval = str[i] - '0';
            else if (str[i] >= 'A' && str[i] <= 'F')
                hexval = str[i] - 'A' + 10;
            else if (str[i] >= 'a' && str[i] <= 'f')
                hexval = str[i] - 'a' + 10;
            else
                break;
            val = (val << 4) + hexval;
        }
        /* handle #rgb case */
        if (i == 4) {
            val = ((val & 0xf00) << 8) | ((val & 0x0f0) << 4) | (val & 0x00f);
            val |= val << 4;
        }

        val |= 0xff000000; /* opaque */
    }
    else if (g_str_has_prefix (str, "rgb")) {
        gint r, g, b, a;
        gboolean has_alpha;
        guint nb_toks;
        char **toks;

        r = g = b = 0;
        a = 255;

        if (str[3] == 'a') {
            /* "rgba" */
            has_alpha = TRUE;
            str += 4;
        }
        else {
            /* "rgb" */
            has_alpha = FALSE;
            str += 3;
        }

        str = strchr (str, '(');
        if (str == NULL)
          return val;

        toks = hisvg_css_parse_list (str + 1, &nb_toks);

        if (toks) {
            if (nb_toks == (has_alpha ? 4 : 3)) {
                r = hisvg_css_clip_rgb_percent (toks[0], 255.0);
                g = hisvg_css_clip_rgb_percent (toks[1], 255.0);
                b = hisvg_css_clip_rgb_percent (toks[2], 255.0);
                if (has_alpha)
                    a = hisvg_css_clip_rgb_percent (toks[3], 1.0);
                else
                    a = 255;
            }

            g_strfreev (toks);
        }

        val = PACK_RGBA (r, g, b, a);
    } else if (!strcmp (str, "inherit"))
        UNSETINHERIT ();
    else {
// TODO: xue
#if 0
        CRRgb rgb;

        if (cr_rgb_set_from_name (&rgb, (const guchar *) str) == CR_OK) {
            val = PACK_RGB (rgb.red, rgb.green, rgb.blue);
        } else {
            /* default to opaque black on failed lookup */
            UNSETINHERIT ();
            val = PACK_RGB (0, 0, 0);
        }
#endif
    }

    return val;
}

#undef PACK_RGB
#undef PACK_RGBA

guint
hisvg_css_parse_opacity (const char *str)
{
    char *end_ptr = NULL;
    double opacity;

    opacity = g_ascii_strtod (str, &end_ptr);

    if (((opacity == -HUGE_VAL || opacity == HUGE_VAL) && (ERANGE == errno)) ||
        *end_ptr != '\0')
        opacity = 1.;

    opacity = CLAMP (opacity, 0., 1.);

    return (guint) floor (opacity * 255. + 0.5);
}

/*
  <angle>: An angle value is a <number>  optionally followed immediately with 
  an angle unit identifier. Angle unit identifiers are:

    * deg: degrees
    * grad: grads
    * rad: radians

    For properties defined in [CSS2], an angle unit identifier must be provided.
    For SVG-specific attributes and properties, the angle unit identifier is 
    optional. If not provided, the angle value is assumed to be in degrees.
*/
double
hisvg_css_parse_angle (const char *str)
{
    double degrees;
    char *end_ptr;

    degrees = g_ascii_strtod (str, &end_ptr);

    /* todo: error condition - figure out how to best represent it */
    if ((degrees == -HUGE_VAL || degrees == HUGE_VAL) && (ERANGE == errno))
        return 0.0;

    if (end_ptr) {
        if (!strcmp (end_ptr, "rad"))
            return degrees * 180. / G_PI;
        else if (!strcmp (end_ptr, "grad"))
            return degrees * 360. / 400.;
    }

    return degrees;
}

/*
  <frequency>: Frequency values are used with aural properties. The normative 
  definition of frequency values can be found in [CSS2-AURAL]. A frequency 
  value is a <number> immediately followed by a frequency unit identifier. 
  Frequency unit identifiers are:

    * Hz: Hertz
    * kHz: kilo Hertz

    Frequency values may not be negative.
*/
double
hisvg_css_parse_frequency (const char *str)
{
    double f_hz;
    char *end_ptr;

    f_hz = g_ascii_strtod (str, &end_ptr);

    /* todo: error condition - figure out how to best represent it */
    if ((f_hz == -HUGE_VAL || f_hz == HUGE_VAL) && (ERANGE == errno))
        return 0.0;

    if (end_ptr && !strcmp (end_ptr, "kHz"))
        return f_hz * 1000.;

    return f_hz;
}

/*
  <time>: A time value is a <number> immediately followed by a time unit 
  identifier. Time unit identifiers are:
  
  * ms: milliseconds
  * s: seconds
  
  Time values are used in CSS properties and may not be negative.
*/
double
hisvg_css_parse_time (const char *str)
{
    double ms;
    char *end_ptr;

    ms = g_ascii_strtod (str, &end_ptr);

    /* todo: error condition - figure out how to best represent it */
    if ((ms == -HUGE_VAL || ms == HUGE_VAL) && (ERANGE == errno))
        return 0.0;

    if (end_ptr && !strcmp (end_ptr, "s"))
        return ms * 1000.;

    return ms;
}

HiSVGTextStyle
hisvg_css_parse_font_style (const char *str, gboolean * inherit)
{
    SETINHERIT ();

    if (str) {
        if (!strcmp (str, "oblique"))
            return HISVG_TEXT_STYLE_OBLIQUE;
        if (!strcmp (str, "italic"))
            return HISVG_TEXT_STYLE_ITALIC;
        if (!strcmp (str, "normal"))
            return HISVG_TEXT_STYLE_NORMAL;
        if (!strcmp (str, "inherit")) {
            UNSETINHERIT ();
            return HISVG_TEXT_STYLE_NORMAL;
        }
    }
    UNSETINHERIT ();
    return HISVG_TEXT_STYLE_NORMAL;
}

HiSVGTextVariant
hisvg_css_parse_font_variant (const char *str, gboolean * inherit)
{
    SETINHERIT ();

    if (str) {
        if (!strcmp (str, "small-caps"))
            return HISVG_TEXT_VARIANT_SMALL_CAPS;
        else if (!strcmp (str, "inherit")) {
            UNSETINHERIT ();
            return HISVG_TEXT_VARIANT_NORMAL;
        }
    }
    UNSETINHERIT ();
    return HISVG_TEXT_VARIANT_NORMAL;
}

HiSVGTextWeight
hisvg_css_parse_font_weight (const char *str, gboolean * inherit)
{
    SETINHERIT ();
    if (str) {
        if (!strcmp (str, "lighter"))
            return HISVG_TEXT_WEIGHT_LIGHT;
        else if (!strcmp (str, "bold"))
            return HISVG_TEXT_WEIGHT_BOLD;
        else if (!strcmp (str, "bolder"))
            return HISVG_TEXT_WEIGHT_ULTRABOLD;
        else if (!strcmp (str, "100"))
            return (HiSVGTextWeight) 100;
        else if (!strcmp (str, "200"))
            return (HiSVGTextWeight) 200;
        else if (!strcmp (str, "300"))
            return (HiSVGTextWeight) 300;
        else if (!strcmp (str, "400"))
            return (HiSVGTextWeight) 400;
        else if (!strcmp (str, "500"))
            return (HiSVGTextWeight) 500;
        else if (!strcmp (str, "600"))
            return (HiSVGTextWeight) 600;
        else if (!strcmp (str, "700"))
            return (HiSVGTextWeight) 700;
        else if (!strcmp (str, "800"))
            return (HiSVGTextWeight) 800;
        else if (!strcmp (str, "900"))
            return (HiSVGTextWeight) 900;
        else if (!strcmp (str, "inherit")) {
            UNSETINHERIT ();
            return HISVG_TEXT_WEIGHT_NORMAL;
        }
    }

    UNSETINHERIT ();
    return HISVG_TEXT_WEIGHT_NORMAL;
}

HiSVGTextStretch
hisvg_css_parse_font_stretch (const char *str, gboolean * inherit)
{
    SETINHERIT ();

    if (str) {
        if (!strcmp (str, "ultra-condensed"))
            return HISVG_TEXT_STRETCH_ULTRA_CONDENSED;
        else if (!strcmp (str, "extra-condensed"))
            return HISVG_TEXT_STRETCH_EXTRA_CONDENSED;
        else if (!strcmp (str, "condensed") || !strcmp (str, "narrower"))       /* narrower not quite correct */
            return HISVG_TEXT_STRETCH_CONDENSED;
        else if (!strcmp (str, "semi-condensed"))
            return HISVG_TEXT_STRETCH_SEMI_CONDENSED;
        else if (!strcmp (str, "semi-expanded"))
            return HISVG_TEXT_STRETCH_SEMI_EXPANDED;
        else if (!strcmp (str, "expanded") || !strcmp (str, "wider"))   /* wider not quite correct */
            return HISVG_TEXT_STRETCH_EXPANDED;
        else if (!strcmp (str, "extra-expanded"))
            return HISVG_TEXT_STRETCH_EXTRA_EXPANDED;
        else if (!strcmp (str, "ultra-expanded"))
            return HISVG_TEXT_STRETCH_ULTRA_EXPANDED;
        else if (!strcmp (str, "inherit")) {
            UNSETINHERIT ();
            return HISVG_TEXT_STRETCH_NORMAL;
        }
    }
    UNSETINHERIT ();
    return HISVG_TEXT_STRETCH_NORMAL;
}

const char *
hisvg_css_parse_font_family (const char *str, gboolean * inherit)
{
    SETINHERIT ();

    if (!str)
        return NULL;
    else if (!strcmp (str, "inherit")) {
        UNSETINHERIT ();
        return NULL;
    } else
        return str;
}

gchar **
hisvg_css_parse_list (const char *in_str, guint * out_list_len)
{
    char *ptr, *tok;
    char *str;

    guint n = 0;
    GSList *string_list = NULL;
    gchar **string_array = NULL;

    str = g_strdup (in_str);
    tok = strtok_r (str, ", \t", &ptr);
    if (tok != NULL) {
        if (strcmp (tok, " ") != 0) {
            string_list = g_slist_prepend (string_list, g_strdup (tok));
            n++;
        }

        while ((tok = strtok_r (NULL, ", \t", &ptr)) != NULL) {
            if (strcmp (tok, " ") != 0) {
                string_list = g_slist_prepend (string_list, g_strdup (tok));
                n++;
            }
        }
    }
    g_free (str);

    if (out_list_len)
        *out_list_len = n;

    if (string_list) {
        GSList *slist;

        string_array = g_new (gchar *, n + 1);

        string_array[n--] = NULL;
        for (slist = string_list; slist; slist = slist->next)
            string_array[n--] = (gchar *) slist->data;

        g_slist_free (string_list);
    }

    return string_array;
}

gdouble *
hisvg_css_parse_number_list (const char *in_str, guint * out_list_len)
{
    gchar **string_array;
    gdouble *output;
    guint len, i;

    if (out_list_len)
        *out_list_len = 0;

    string_array = hisvg_css_parse_list (in_str, &len);

    if (!(string_array && len))
        return NULL;

    output = g_new (gdouble, len);

    /* TODO: some error checking */
    for (i = 0; i < len; i++)
        output[i] = g_ascii_strtod (string_array[i], NULL);

    g_strfreev (string_array);

    if (out_list_len != NULL)
        *out_list_len = len;

    return output;
}

void
hisvg_css_parse_number_optional_number (const char *str, double *x, double *y)
{
    char *endptr;

    /* TODO: some error checking */

    *x = g_ascii_strtod (str, &endptr);

    if (endptr && *endptr != '\0')
        while (g_ascii_isspace (*endptr) && *endptr)
            endptr++;

    if (endptr && *endptr)
        *y = g_ascii_strtod (endptr, NULL);
    else
        *y = *x;
}

int
hisvg_css_parse_aspect_ratio (const char *str)
{
    char **elems;
    guint nb_elems;

    int ratio = HISVG_ASPECT_RATIO_NONE;

    elems = hisvg_css_parse_list (str, &nb_elems);

    if (elems && nb_elems) {
        guint i;

        for (i = 0; i < nb_elems; i++) {
            if (!strcmp (elems[i], "xMinYMin"))
                ratio = HISVG_ASPECT_RATIO_XMIN_YMIN;
            else if (!strcmp (elems[i], "xMidYMin"))
                ratio = HISVG_ASPECT_RATIO_XMID_YMIN;
            else if (!strcmp (elems[i], "xMaxYMin"))
                ratio = HISVG_ASPECT_RATIO_XMAX_YMIN;
            else if (!strcmp (elems[i], "xMinYMid"))
                ratio = HISVG_ASPECT_RATIO_XMIN_YMID;
            else if (!strcmp (elems[i], "xMidYMid"))
                ratio = HISVG_ASPECT_RATIO_XMID_YMID;
            else if (!strcmp (elems[i], "xMaxYMid"))
                ratio = HISVG_ASPECT_RATIO_XMAX_YMID;
            else if (!strcmp (elems[i], "xMinYMax"))
                ratio = HISVG_ASPECT_RATIO_XMIN_YMAX;
            else if (!strcmp (elems[i], "xMidYMax"))
                ratio = HISVG_ASPECT_RATIO_XMID_YMAX;
            else if (!strcmp (elems[i], "xMaxYMax"))
                ratio = HISVG_ASPECT_RATIO_XMAX_YMAX;
            else if (!strcmp (elems[i], "slice"))
                ratio |= HISVG_ASPECT_RATIO_SLICE;
        }

        g_strfreev (elems);
    }

    return ratio;
}

gboolean
hisvg_css_parse_overflow (const char *str, gboolean * inherit)
{
    SETINHERIT ();
    if (!strcmp (str, "visible") || !strcmp (str, "auto"))
        return 1;
    if (!strcmp (str, "hidden") || !strcmp (str, "scroll"))
        return 0;
    UNSETINHERIT ();
    return 0;
}

static void
hisvg_xml_noerror (void *data, xmlErrorPtr error)
{
}

/* This is quite hacky and not entirely correct, but apparently 
 * libxml2 has NO support for parsing pseudo attributes as defined 
 * by the xml-styleheet spec.
 */
char **
hisvg_css_parse_xml_attribute_string (const char *attribute_string)
{
    xmlSAXHandler handler;
    xmlParserCtxtPtr parser;
    xmlDocPtr doc;
    xmlNodePtr node;
    xmlAttrPtr attr;
    char *tag;
    GPtrArray *attributes;
    char **retval = NULL;

    tag = g_strdup_printf ("<hisvg-hack %s />\n", attribute_string);

    memset (&handler, 0, sizeof (handler));
    xmlSAX2InitDefaultSAXHandler (&handler, 0);
    handler.serror = hisvg_xml_noerror;
    parser = xmlCreatePushParserCtxt (&handler, NULL, tag, strlen (tag) + 1, NULL);
    parser->options |= XML_PARSE_NONET;

    if (xmlParseDocument (parser) != 0)
        goto done;

    if ((doc = parser->myDoc) == NULL ||
        (node = doc->children) == NULL ||
        strcmp ((const char *) node->name, "hisvg-hack") != 0 ||
        node->next != NULL ||
        node->properties == NULL)
          goto done;

    attributes = g_ptr_array_new ();
    for (attr = node->properties; attr; attr = attr->next) {
        xmlNodePtr content = attr->children;

        g_ptr_array_add (attributes, g_strdup ((char *) attr->name));
        if (content)
          g_ptr_array_add (attributes, g_strdup ((char *) content->content));
        else
          g_ptr_array_add (attributes, g_strdup (""));
    }

    g_ptr_array_add (attributes, NULL);
    retval = (char **) g_ptr_array_free (attributes, FALSE);

  done:
    if (parser->myDoc)
      xmlFreeDoc (parser->myDoc);
    xmlFreeParserCtxt (parser);
    g_free (tag);

    return retval;
}
