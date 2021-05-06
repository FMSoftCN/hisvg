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

#define _GNU_SOURCE 1

#include "hisvg-common.h"
#include "hisvg-private.h"
#include "hisvg-css.h"
#include "hisvg-styles.h"
#include "hisvg-shapes.h"
#include "hisvg-structure.h"
#include "hisvg-image.h"
#include "hisvg-io.h"
#include "hisvg-text.h"
#include "hisvg-filter.h"
#include "hisvg-mask.h"
#include "hisvg-marker.h"
#include "hisvg-cairo-render.h"
#include "hisvg-select.h"

#include <libxml/uri.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>

#include <gio/gio.h>

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdlib.h>

#include "hisvg-path.h"
#include "hisvg-paint-server.h"
#include "hisvg-xml.h"

#ifdef G_OS_WIN32
static char *
hisvg_realpath_utf8 (const char *filename, const char *unused)
{
    wchar_t *wfilename;
    wchar_t *wfull;
    char *full;

    wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
    if (!wfilename)
        return NULL;

    wfull = _wfullpath (NULL, wfilename, 0);
    g_free (wfilename);
    if (!wfull)
        return NULL;

    full = g_utf16_to_utf8 (wfull, -1, NULL, NULL, NULL);
    free (wfull);

    if (!full)
        return NULL;

    return full;
}

#define realpath(a,b) hisvg_realpath_utf8 (a, b)
#endif

/*
 * This is configurable at runtime
 */
#define HISVG_DEFAULT_DPI_X 90.0
#define HISVG_DEFAULT_DPI_Y 90.0

G_GNUC_INTERNAL
double hisvg_internal_dpi_x = HISVG_DEFAULT_DPI_X;
G_GNUC_INTERNAL
double hisvg_internal_dpi_y = HISVG_DEFAULT_DPI_Y;

static xmlSAXHandler hisvgSAXHandlerStruct;
static gboolean hisvgSAXHandlerStructInited = FALSE;

typedef struct _HiSVGSaxHandlerDefs {
    HiSVGSaxHandler super;
    HiSVGHandle *ctx;
} HiSVGSaxHandlerDefs;

typedef struct _HiSVGSaxHandlerStyle {
    HiSVGSaxHandler super;
    HiSVGSaxHandlerDefs *parent;
    HiSVGHandle *ctx;
    GString *style;
    gboolean is_text_css;
} HiSVGSaxHandlerStyle;

typedef struct {
    HiSVGSaxHandler super;
    HiSVGHandle *ctx;
    const char *name;
    GString *string;
    GString **stringptr;
} HiSVGSaxHandlerExtra;

/* hide this fact from the general public */
typedef HiSVGSaxHandlerExtra HiSVGSaxHandlerTitle;
typedef HiSVGSaxHandlerExtra HiSVGSaxHandlerDesc;
typedef HiSVGSaxHandlerExtra HiSVGSaxHandlerMetadata;

static void
hisvg_style_handler_free (HiSVGSaxHandler * self)
{
    HiSVGSaxHandlerStyle *z = (HiSVGSaxHandlerStyle *) self;
    HiSVGHandle *ctx = z->ctx;

    if (z->is_text_css)
        hisvg_parse_cssbuffer (ctx, z->style->str, z->style->len);

    g_string_free (z->style, TRUE);
    g_free (z);
}

static void
hisvg_style_handler_characters (HiSVGSaxHandler * self, const char *ch, int len)
{
    HiSVGSaxHandlerStyle *z = (HiSVGSaxHandlerStyle *) self;
    g_string_append_len (z->style, ch, len);
}

static void
hisvg_style_handler_start (HiSVGSaxHandler * self, const char *name, HiSVGPropertyBag * atts)
{
}

static void
hisvg_style_handler_end (HiSVGSaxHandler * self, const char *name)
{
    HiSVGSaxHandlerStyle *z = (HiSVGSaxHandlerStyle *) self;
    HiSVGHandle *ctx = z->ctx;
    HiSVGSaxHandler *prev = &z->parent->super;

    if (!strcmp (name, "style")) {
        if (ctx->priv->handler != NULL) {
            ctx->priv->handler->free (ctx->priv->handler);
            ctx->priv->handler = prev;
        }
    }
}

static void
hisvg_start_style (HiSVGHandle * ctx, HiSVGPropertyBag *atts)
{
    HiSVGSaxHandlerStyle *handler = g_new0 (HiSVGSaxHandlerStyle, 1);
    const char *type;

    type = hisvg_property_bag_lookup (atts, "type");

    handler->super.free = hisvg_style_handler_free;
    handler->super.characters = hisvg_style_handler_characters;
    handler->super.start_element = hisvg_style_handler_start;
    handler->super.end_element = hisvg_style_handler_end;
    handler->ctx = ctx;

    handler->style = g_string_new (NULL);
    handler->is_text_css = type && g_ascii_strcasecmp (type, "text/css") == 0;

    handler->parent = (HiSVGSaxHandlerDefs *) ctx->priv->handler;
    ctx->priv->handler = &handler->super;
}


static void
hisvg_standard_element_start (HiSVGHandle * ctx, const char *name, HiSVGPropertyBag * atts)
{
    /*replace this stuff with a hash for fast reading! */
    HiSVGNode *newnode = NULL;
    if (!strcmp (name, "g"))
        newnode = hisvg_new_group ("g");
    else if (!strcmp (name, "a"))       /*treat anchors as groups for now */
        newnode = hisvg_new_group ("g");
    else if (!strcmp (name, "switch"))
        newnode = hisvg_new_switch (name);
    else if (!strcmp (name, "defs"))
        newnode = hisvg_new_defs (name);
    else if (!strcmp (name, "use"))
        newnode = hisvg_new_use (name);
    else if (!strcmp (name, "path"))
        newnode = hisvg_new_path (name);
    else if (!strcmp (name, "line"))
        newnode = hisvg_new_line (name);
    else if (!strcmp (name, "rect"))
        newnode = hisvg_new_rect (name);
    else if (!strcmp (name, "ellipse"))
        newnode = hisvg_new_ellipse (name);
    else if (!strcmp (name, "circle"))
        newnode = hisvg_new_circle (name);
    else if (!strcmp (name, "polygon"))
        newnode = hisvg_new_polygon (name);
    else if (!strcmp (name, "polyline"))
        newnode = hisvg_new_polyline (name);
    else if (!strcmp (name, "symbol"))
        newnode = hisvg_new_symbol (name);
    else if (!strcmp (name, "svg"))
        newnode = hisvg_new_svg (name);
    else if (!strcmp (name, "mask"))
        newnode = hisvg_new_mask (name);
    else if (!strcmp (name, "clipPath"))
        newnode = hisvg_new_clip_path (name);
    else if (!strcmp (name, "image"))
        newnode = hisvg_new_image (name);
    else if (!strcmp (name, "marker"))
        newnode = hisvg_new_marker (name);
    else if (!strcmp (name, "stop"))
        newnode = hisvg_new_stop (name);
    else if (!strcmp (name, "pattern"))
        newnode = hisvg_new_pattern (name);
    else if (!strcmp (name, "linearGradient"))
        newnode = hisvg_new_linear_gradient (name);
    else if (!strcmp (name, "radialGradient"))
        newnode = hisvg_new_radial_gradient (name);
    else if (!strcmp (name, "conicalGradient"))
        newnode = hisvg_new_radial_gradient (name);
    else if (!strcmp (name, "filter"))
        newnode = hisvg_new_filter (name);
    else if (!strcmp (name, "feBlend"))
        newnode = hisvg_new_filter_primitive_blend (name);
    else if (!strcmp (name, "feColorMatrix"))
        newnode = hisvg_new_filter_primitive_color_matrix (name);
    else if (!strcmp (name, "feComponentTransfer"))
        newnode = hisvg_new_filter_primitive_component_transfer (name);
    else if (!strcmp (name, "feComposite"))
        newnode = hisvg_new_filter_primitive_composite (name);
    else if (!strcmp (name, "feConvolveMatrix"))
        newnode = hisvg_new_filter_primitive_convolve_matrix (name);
    else if (!strcmp (name, "feDiffuseLighting"))
        newnode = hisvg_new_filter_primitive_diffuse_lighting (name);
    else if (!strcmp (name, "feDisplacementMap"))
        newnode = hisvg_new_filter_primitive_displacement_map (name);
    else if (!strcmp (name, "feFlood"))
        newnode = hisvg_new_filter_primitive_flood (name);
    else if (!strcmp (name, "feGaussianBlur"))
        newnode = hisvg_new_filter_primitive_gaussian_blur (name);
    else if (!strcmp (name, "feImage"))
        newnode = hisvg_new_filter_primitive_image (name);
    else if (!strcmp (name, "feMerge"))
        newnode = hisvg_new_filter_primitive_merge (name);
    else if (!strcmp (name, "feMorphology"))
        newnode = hisvg_new_filter_primitive_erode (name);
    else if (!strcmp (name, "feOffset"))
        newnode = hisvg_new_filter_primitive_offset (name);
    else if (!strcmp (name, "feSpecularLighting"))
        newnode = hisvg_new_filter_primitive_specular_lighting (name);
    else if (!strcmp (name, "feTile"))
        newnode = hisvg_new_filter_primitive_tile (name);
    else if (!strcmp (name, "feTurbulence"))
        newnode = hisvg_new_filter_primitive_turbulence (name);
    else if (!strcmp (name, "feMergeNode"))
        newnode = hisvg_new_filter_primitive_merge_node (name);
    else if (!strcmp (name, "feFuncR"))
        newnode = hisvg_new_node_component_transfer_function (name, 'r'); /* See hisvg_filter_primitive_component_transfer_render() for where these values are used */
    else if (!strcmp (name, "feFuncG"))
        newnode = hisvg_new_node_component_transfer_function (name, 'g');
    else if (!strcmp (name, "feFuncB"))
        newnode = hisvg_new_node_component_transfer_function (name, 'b');
    else if (!strcmp (name, "feFuncA"))
        newnode = hisvg_new_node_component_transfer_function (name, 'a');
    else if (!strcmp (name, "feDistantLight"))
        newnode = hisvg_new_node_light_source (name, 'd');
    else if (!strcmp (name, "feSpotLight"))
        newnode = hisvg_new_node_light_source (name, 's');
    else if (!strcmp (name, "fePointLight"))
        newnode = hisvg_new_node_light_source (name, 'p');
    /* hack to make multiImage sort-of work */
    else if (!strcmp (name, "multiImage"))
        newnode = hisvg_new_switch (name);
    else if (!strcmp (name, "subImageRef"))
        newnode = hisvg_new_image (name);
    else if (!strcmp (name, "subImage"))
        newnode = hisvg_new_group ("g");
    else if (!strcmp (name, "text"))
        newnode = hisvg_new_text (name);
    else if (!strcmp (name, "tspan"))
        newnode = hisvg_new_tspan (name);
    else if (!strcmp (name, "tref"))
        newnode = hisvg_new_tref (name);
    else {
		/* hack for bug 401115. whenever we encounter a node we don't understand, push it into a group. 
		   this will allow us to handle things like conditionals properly. */
		newnode = hisvg_new_group ("g");
	}

    if (newnode) {
        g_assert (HISVG_NODE_TYPE (newnode) != HISVG_NODE_TYPE_INVALID);
        hisvg_node_set_atts (newnode, ctx, atts);
        hisvg_defs_register_memory (ctx->priv->defs, newnode);
        if (ctx->priv->currentnode) {
            hisvg_node_group_pack (ctx->priv->currentnode, newnode);
            ctx->priv->currentnode = newnode;
        } else if (HISVG_NODE_TYPE (newnode) == HISVG_NODE_TYPE_SVG) {
            ctx->priv->treebase = newnode;
            ctx->priv->currentnode = newnode;
        }
    }
}

/* extra (title, desc, metadata) */

static void
hisvg_extra_handler_free (HiSVGSaxHandler * self)
{
    HiSVGSaxHandlerExtra *z = (HiSVGSaxHandlerExtra *) self;

    if (z->stringptr) {
        if (*z->stringptr)
            g_string_free (*z->stringptr, TRUE);
        *z->stringptr = z->string;
    } else if (z->string) {
        g_string_free (z->string, TRUE);
    }

    g_free (self);
}

static void
hisvg_extra_handler_characters (HiSVGSaxHandler * self, const char *ch, int len)
{
    HiSVGSaxHandlerExtra *z = (HiSVGSaxHandlerExtra *) self;

    /* This isn't quite the correct behavior - in theory, any graphics
       element may contain a title, desc, or metadata element */

    if (!z->string)
        return;

    if (!ch || !len)
        return;

    if (!g_utf8_validate ((char *) ch, len, NULL)) {
        char *utf8;
        utf8 = hisvg_make_valid_utf8 ((char *) ch, len);
        g_string_append (z->string, utf8);
        g_free (utf8);
    } else {
        g_string_append_len (z->string, (char *) ch, len);
    }
}

static void
hisvg_extra_handler_start (HiSVGSaxHandler * self, const char *name, HiSVGPropertyBag * atts)
{
}

static void
hisvg_extra_handler_end (HiSVGSaxHandler * self, const char *name)
{
    HiSVGSaxHandlerExtra *z = (HiSVGSaxHandlerExtra *) self;
    HiSVGHandle *ctx = z->ctx;

    if (!strcmp (name, z->name)) {
        if (ctx->priv->handler != NULL) {
            ctx->priv->handler->free (ctx->priv->handler);
            ctx->priv->handler = NULL;
        }
    }
}

static HiSVGSaxHandlerExtra *
hisvg_start_extra (HiSVGHandle * ctx,
                  const char *name,
                  GString **stringptr)
{
    HiSVGSaxHandlerExtra *handler = g_new0 (HiSVGSaxHandlerExtra, 1);
    HiSVGNode *treebase = ctx->priv->treebase;
    HiSVGNode *currentnode = ctx->priv->currentnode;
    gboolean do_care;

    /* only parse <extra> for the <svg> node.
     * This isn't quite the correct behavior - any graphics
     * element may contain a <extra> element.
     */
    do_care = treebase != NULL && treebase == currentnode;

    handler->super.free = hisvg_extra_handler_free;
    handler->super.characters = hisvg_extra_handler_characters;
    handler->super.start_element = hisvg_extra_handler_start;
    handler->super.end_element = hisvg_extra_handler_end;
    handler->ctx = ctx;
    handler->name = name; /* interned */
    handler->string = do_care ? g_string_new (NULL) : NULL;
    handler->stringptr = do_care ? stringptr : NULL;

    ctx->priv->handler = &handler->super;

    return handler;
}

/* start desc */

static void
hisvg_start_desc (HiSVGHandle * ctx)
{
    hisvg_start_extra (ctx, "desc", &ctx->priv->desc);
}

/* end desc */

/* start title */

static void
hisvg_start_title (HiSVGHandle * ctx)
{
    hisvg_start_extra (ctx, "title", &ctx->priv->title);
}

/* end title */

/* start metadata */

static void
hisvg_metadata_props_enumerate (const char *key, const char *value, gpointer user_data)
{
    GString *metadata = (GString *) user_data;
    g_string_append_printf (metadata, "%s=\"%s\" ", key, value);
}

static void
hisvg_metadata_handler_start (HiSVGSaxHandler * self, const char *name, HiSVGPropertyBag * atts)
{
    HiSVGSaxHandlerMetadata *z = (HiSVGSaxHandlerMetadata *) self;

    hisvg_extra_handler_start (self, name, atts);

    if (!z->string)
        return;

    g_string_append_printf (z->string, "<%s ", name);
    hisvg_property_bag_enumerate (atts, hisvg_metadata_props_enumerate, z->string);
    g_string_append (z->string, ">\n");
}

static void
hisvg_metadata_handler_end (HiSVGSaxHandler * self, const char *name)
{
    HiSVGSaxHandlerMetadata *z = (HiSVGSaxHandlerMetadata *) self;

    if (strcmp (name, z->name) != 0) {
        if (z->string)
            g_string_append_printf (z->string, "</%s>\n", name);
    } else {
        hisvg_extra_handler_end (self, name);
    }
}

static void
hisvg_start_metadata (HiSVGHandle * ctx)
{
    HiSVGSaxHandlerMetadata *handler = hisvg_start_extra (ctx, "metadata", &ctx->priv->metadata);

    handler->super.start_element = hisvg_metadata_handler_start;
    handler->super.end_element = hisvg_metadata_handler_end;
}

/* end metadata */

/* start xinclude */

typedef struct _HiSVGSaxHandlerXinclude {
    HiSVGSaxHandler super;

    HiSVGSaxHandler *prev_handler;
    HiSVGHandle *ctx;
    gboolean success;
    gboolean in_fallback;
} HiSVGSaxHandlerXinclude;

static void
 hisvg_start_xinclude (HiSVGHandle * ctx, HiSVGPropertyBag * atts);
static void
 hisvg_characters_impl (HiSVGHandle * ctx, const xmlChar * ch, int len);

static void
hisvg_xinclude_handler_free (HiSVGSaxHandler * self)
{
    g_free (self);
}

static void
hisvg_xinclude_handler_characters (HiSVGSaxHandler * self, const char *ch, int len)
{
    HiSVGSaxHandlerXinclude *z = (HiSVGSaxHandlerXinclude *) self;

    if (z->in_fallback) {
        hisvg_characters_impl (z->ctx, (const xmlChar *) ch, len);
    }
}

static void
hisvg_xinclude_handler_start (HiSVGSaxHandler * self, const char *name, HiSVGPropertyBag * atts)
{
    HiSVGSaxHandlerXinclude *z = (HiSVGSaxHandlerXinclude *) self;

    if (!z->success) {
        if (z->in_fallback) {
            if (!strcmp (name, "xi:include"))
                hisvg_start_xinclude (z->ctx, atts);
            else
                hisvg_standard_element_start (z->ctx, (const char *) name, atts);
        } else if (!strcmp (name, "xi:fallback")) {
            z->in_fallback = TRUE;
        }
    }
}

static void
hisvg_xinclude_handler_end (HiSVGSaxHandler * self, const char *name)
{
    HiSVGSaxHandlerXinclude *z = (HiSVGSaxHandlerXinclude *) self;
    HiSVGHandle *ctx = z->ctx;

    if (!strcmp (name, "include") || !strcmp (name, "xi:include")) {
        if (ctx->priv->handler != NULL) {
            HiSVGSaxHandler *previous_handler;

            previous_handler = z->prev_handler;
            ctx->priv->handler->free (ctx->priv->handler);
            ctx->priv->handler = previous_handler;
        }
    } else if (z->in_fallback) {
        if (!strcmp (name, "xi:fallback"))
            z->in_fallback = FALSE;
    }
}

static void
_hisvg_set_xml_parse_options(xmlParserCtxtPtr xml_parser,
                            HiSVGHandle *ctx)
{
    xml_parser->options |= XML_PARSE_NONET;

    if (ctx->priv->flags & HISVG_HANDLE_FLAG_UNLIMITED) {
#if LIBXML_VERSION > 20632
        xml_parser->options |= XML_PARSE_HUGE;
#endif
    }

#if LIBXML_VERSION > 20800
    xml_parser->options |= XML_PARSE_BIG_LINES;
#endif
}

/* http://www.w3.org/TR/xinclude/ */
static void
hisvg_start_xinclude (HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    HiSVGSaxHandlerXinclude *handler;
    const char *href, *parse;
    gboolean success = FALSE;

    href = hisvg_property_bag_lookup (atts, "href");
    if (href == NULL)
        goto fallback;

    parse = hisvg_property_bag_lookup (atts, "parse");
    if (parse && !strcmp (parse, "text")) {
        char *data;
        gsize data_len;
        const char *encoding;

        data = _hisvg_handle_acquire_data (ctx, href, NULL, &data_len, NULL);
        if (data == NULL)
            goto fallback;

        encoding = hisvg_property_bag_lookup (atts, "encoding");
        if (encoding && g_ascii_strcasecmp (encoding, "UTF-8") != 0) {
            char *text_data;
            gsize text_data_len;

            text_data = g_convert (data, data_len, "utf-8", encoding, NULL,
                                   &text_data_len, NULL);
            g_free (data);

            data = text_data;
            data_len = text_data_len;
        }

        hisvg_characters_impl (ctx, (xmlChar *) data, data_len);

        g_free (data);
    } else {
        /* xml */
        GInputStream *stream;
        GError *err = NULL;
        xmlDocPtr xml_doc;
        xmlParserCtxtPtr xml_parser;
        xmlParserInputBufferPtr buffer;
        xmlParserInputPtr input;

        stream = _hisvg_handle_acquire_stream (ctx, href, NULL, NULL);
        if (stream == NULL)
            goto fallback;

        xml_parser = xmlCreatePushParserCtxt (&hisvgSAXHandlerStruct, ctx, NULL, 0, NULL);
        _hisvg_set_xml_parse_options(xml_parser, ctx);

        buffer = _hisvg_xml_input_buffer_new_from_stream (stream, NULL /* cancellable */, XML_CHAR_ENCODING_NONE, &err);
        g_object_unref (stream);

        input = xmlNewIOInputStream (xml_parser, buffer /* adopts */, XML_CHAR_ENCODING_NONE);

        if (xmlPushInput (xml_parser, input) < 0) {
            g_clear_error (&err);
            xmlFreeInputStream (input);
            xmlFreeParserCtxt (xml_parser);
            goto fallback;
        }

        (void) xmlParseDocument (xml_parser);

        xml_doc = xml_parser->myDoc;
        xmlFreeParserCtxt (xml_parser);
        if (xml_doc)
            xmlFreeDoc (xml_doc);

        g_clear_error (&err);
    }

    success = TRUE;

  fallback:

    /* needed to handle xi:fallback */
    handler = g_new0 (HiSVGSaxHandlerXinclude, 1);

    handler->super.free = hisvg_xinclude_handler_free;
    handler->super.characters = hisvg_xinclude_handler_characters;
    handler->super.start_element = hisvg_xinclude_handler_start;
    handler->super.end_element = hisvg_xinclude_handler_end;
    handler->prev_handler = ctx->priv->handler;
    handler->ctx = ctx;
    handler->success = success;

    ctx->priv->handler = &handler->super;
}

/* end xinclude */

static void
hisvg_start_element (void *data, const xmlChar * name, const xmlChar ** atts)
{
    HiSVGPropertyBag *bag;
    HiSVGHandle *ctx = (HiSVGHandle *) data;

    bag = hisvg_property_bag_new ((const char **) atts);

    if (ctx->priv->handler) {
        ctx->priv->handler_nest++;
        if (ctx->priv->handler->start_element != NULL)
            ctx->priv->handler->start_element (ctx->priv->handler, (const char *) name, bag);
    } else {
        const char *tempname;
        for (tempname = (const char *) name; *tempname != '\0'; tempname++)
            if (*tempname == ':')
                name = (const xmlChar *) (tempname + 1);

        if (!strcmp ((const char *) name, "style"))
            hisvg_start_style (ctx, bag);
        else if (!strcmp ((const char *) name, "title"))
            hisvg_start_title (ctx);
        else if (!strcmp ((const char *) name, "desc"))
            hisvg_start_desc (ctx);
        else if (!strcmp ((const char *) name, "metadata"))
            hisvg_start_metadata (ctx);
        else if (!strcmp ((const char *) name, "include"))      /* xi:include */
            hisvg_start_xinclude (ctx, bag);
        else
            hisvg_standard_element_start (ctx, (const char *) name, bag);
    }

    hisvg_property_bag_free (bag);
}

static void
hisvg_end_element (void *data, const xmlChar * name)
{
    HiSVGHandle *ctx = (HiSVGHandle *) data;

    if (ctx->priv->handler_nest > 0 && ctx->priv->handler != NULL) {
        if (ctx->priv->handler->end_element != NULL)
            ctx->priv->handler->end_element (ctx->priv->handler, (const char *) name);
        ctx->priv->handler_nest--;
    } else {
        const char *tempname;
        for (tempname = (const char *) name; *tempname != '\0'; tempname++)
            if (*tempname == ':')
                name = (const xmlChar *) (tempname + 1);

        if (ctx->priv->handler != NULL) {
            ctx->priv->handler->free (ctx->priv->handler);
            ctx->priv->handler = NULL;
        }

        if (ctx->priv->currentnode &&
            !strcmp ((const char *) name, HISVG_NODE_TAG_NAME(ctx->priv->currentnode)))
        {
                hisvg_pop_def_group (ctx);
        }

        /* FIXMEchpe: shouldn't this check that currentnode == treebase or sth like that? */
        if (ctx->priv->treebase && !strcmp ((const char *)name, "svg"))
            _hisvg_node_svg_apply_atts ((HiSVGNodeSvg *)ctx->priv->treebase, ctx);
    }
}

static void
_hisvg_node_chars_free (HiSVGNode * node)
{
    HiSVGNodeChars *self = (HiSVGNodeChars *) node;
    g_string_free (self->contents, TRUE);
    _hisvg_node_free (node);
}

static HiSVGNodeChars *
hisvg_new_node_chars (const char *text,
                     int len)
{
    HiSVGNodeChars *self;

    self = g_new (HiSVGNodeChars, 1);
    _hisvg_node_init (&self->super, HISVG_NODE_TYPE_CHARS, "chars");

    if (!g_utf8_validate (text, len, NULL)) {
        char *utf8;
        utf8 = hisvg_make_valid_utf8 (text, len);
        self->contents = g_string_new (utf8);
        g_free (utf8);
    } else {
        self->contents = g_string_new_len (text, len);
    }

    self->super.free = _hisvg_node_chars_free;
    self->super.state->cond_true = FALSE;

    return self;
}

static void
hisvg_characters_impl (HiSVGHandle * ctx, const xmlChar * ch, int len)
{
    HiSVGNodeChars *self;

    if (!ch || !len)
        return;

    if (ctx->priv->currentnode) {
        HiSVGNodeType type = HISVG_NODE_TYPE (ctx->priv->currentnode);
        if (type == HISVG_NODE_TYPE_TSPAN ||
            type == HISVG_NODE_TYPE_TEXT) {

            /* find the last CHARS node in the text or tspan node, so that we
               can coalesce the text, and thus avoid screwing up the Pango layouts */
            self = NULL;
            HLDomElementNode* child = HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(ctx->priv->currentnode->base);
            while(child)
            {
                HiSVGNode *node = HISVG_NODE_FROM_DOM_NODE (child);
                child = HISVG_DOM_ELEMENT_NODE_NEXT(child);
                if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_CHARS) {
                    self = (HiSVGNodeChars*)node;
                }
                else if (HISVG_NODE_TYPE (node) == HISVG_NODE_TYPE_TSPAN) {
                    self = NULL;
                }
            }

            if (self != NULL) {
                if (!g_utf8_validate ((char *) ch, len, NULL)) {
                    char *utf8;
                    utf8 = hisvg_make_valid_utf8 ((char *) ch, len);
                    g_string_append (self->contents, utf8);
                    g_free (utf8);
                } else {
                    g_string_append_len (self->contents, (char *)ch, len);
                }

                return;
            }
        }
    }

    self = hisvg_new_node_chars ((char *) ch, len);

    hisvg_defs_register_memory (ctx->priv->defs, (HiSVGNode *) self);
    if (ctx->priv->currentnode)
        hisvg_node_group_pack (ctx->priv->currentnode, (HiSVGNode *) self);
}

static void
hisvg_characters (void *data, const xmlChar * ch, int len)
{
    HiSVGHandle *ctx = (HiSVGHandle *) data;

    if (ctx->priv->handler && ctx->priv->handler->characters != NULL) {
        ctx->priv->handler->characters (ctx->priv->handler, (const char *) ch, len);
        return;
    }

    hisvg_characters_impl (ctx, ch, len);
}

static xmlEntityPtr
hisvg_get_entity (void *data, const xmlChar * name)
{
    HiSVGHandle *ctx = (HiSVGHandle *) data;
    xmlEntityPtr entity;

    entity = g_hash_table_lookup (ctx->priv->entities, name);

    return entity;
}

static void
hisvg_entity_decl (void *data, const xmlChar * name, int type,
                  const xmlChar * publicId, const xmlChar * systemId, xmlChar * content)
{
    HiSVGHandle *ctx = (HiSVGHandle *) data;
    GHashTable *entities = ctx->priv->entities;
    xmlEntityPtr entity;
    xmlChar *resolvedSystemId = NULL, *resolvedPublicId = NULL;

    if (systemId)
        resolvedSystemId = xmlBuildRelativeURI (systemId, (xmlChar*) hisvg_handle_get_base_uri (ctx));
    else if (publicId)
        resolvedPublicId = xmlBuildRelativeURI (publicId, (xmlChar*) hisvg_handle_get_base_uri (ctx));

    if (type == XML_EXTERNAL_PARAMETER_ENTITY && !content) {
        char *entity_data;
        gsize entity_data_len;

        if (systemId)
            entity_data = _hisvg_handle_acquire_data (ctx,
                                                     (const char *) systemId,
                                                     NULL,
                                                     &entity_data_len,
                                                     NULL);
        else if (publicId)
            entity_data = _hisvg_handle_acquire_data (ctx,
                                                     (const char *) publicId,
                                                     NULL,
                                                     &entity_data_len,
                                                     NULL);
        else
            entity_data = NULL;

        if (entity_data) {
            content = xmlCharStrndup (entity_data, entity_data_len);
            g_free (entity_data);
        }
    }

    entity = xmlNewEntity(NULL, name, type, resolvedPublicId, resolvedSystemId, content);

    xmlFree(resolvedPublicId);
    xmlFree(resolvedSystemId);

    g_hash_table_insert (entities, g_strdup ((const char*) name), entity);
}

static void
hisvg_unparsed_entity_decl (void *ctx,
                           const xmlChar * name,
                           const xmlChar * publicId,
                           const xmlChar * systemId, const xmlChar * notationName)
{
    hisvg_entity_decl (ctx, name, XML_INTERNAL_GENERAL_ENTITY, publicId, systemId, NULL);
}

static xmlEntityPtr
hisvg_get_parameter_entity (void *data, const xmlChar * name)
{
    HiSVGHandle *ctx = (HiSVGHandle *) data;
    xmlEntityPtr entity;

    entity = g_hash_table_lookup (ctx->priv->entities, name);

    return entity;
}

static void
hisvg_error_cb (void *data, const char *msg, ...)
{
#ifdef G_ENABLE_DEBUG
    va_list args;

    va_start (args, msg);
    vfprintf (stderr, msg, args);
    va_end (args);
#endif
}

static void
hisvg_processing_instruction (void *ctx, const xmlChar * target, const xmlChar * data)
{
    /* http://www.w3.org/TR/xml-stylesheet/ */
    HiSVGHandle *handle = (HiSVGHandle *) ctx;

    if (!strcmp ((const char *) target, "xml-stylesheet")) {
        HiSVGPropertyBag *atts;
        char **xml_atts;

        xml_atts = hisvg_css_parse_xml_attribute_string ((const char *) data);

        if (xml_atts) {
            const char *value;

            atts = hisvg_property_bag_new ((const char **) xml_atts);
            value = hisvg_property_bag_lookup (atts, "alternate");
            if (!value || !value[0] || (strcmp (value, "no") != 0)) {
                value = hisvg_property_bag_lookup (atts, "type");
                if (value && strcmp (value, "text/css") == 0) {
                    value = hisvg_property_bag_lookup (atts, "href");
                    if (value && value[0]) {
                        char *style_data;
                        gsize style_data_len;
                        char *mime_type = NULL;

                        style_data = _hisvg_handle_acquire_data (handle,
                                                                value,
                                                                &mime_type,
                                                                &style_data_len,
                                                                NULL);
                        if (style_data && 
                            mime_type &&
                            strcmp (mime_type, "text/css") == 0) {
                            hisvg_parse_cssbuffer (handle, style_data, style_data_len);
                        }

                        g_free (mime_type);
                        g_free (style_data);
                    }
                }
            }

            hisvg_property_bag_free (atts);
            g_strfreev (xml_atts);
        }
    }
}

void
hisvg_SAX_handler_struct_init (void)
{
    if (!hisvgSAXHandlerStructInited) {
        hisvgSAXHandlerStructInited = TRUE;

        memset (&hisvgSAXHandlerStruct, 0, sizeof (hisvgSAXHandlerStruct));

        hisvgSAXHandlerStruct.getEntity = hisvg_get_entity;
        hisvgSAXHandlerStruct.entityDecl = hisvg_entity_decl;
        hisvgSAXHandlerStruct.unparsedEntityDecl = hisvg_unparsed_entity_decl;
        hisvgSAXHandlerStruct.getParameterEntity = hisvg_get_parameter_entity;
        hisvgSAXHandlerStruct.characters = hisvg_characters;
        hisvgSAXHandlerStruct.error = hisvg_error_cb;
        hisvgSAXHandlerStruct.cdataBlock = hisvg_characters;
        hisvgSAXHandlerStruct.startElement = hisvg_start_element;
        hisvgSAXHandlerStruct.endElement = hisvg_end_element;
        hisvgSAXHandlerStruct.processingInstruction = hisvg_processing_instruction;
    }
}

/* http://www.ietf.org/rfc/rfc2396.txt */

static gboolean
hisvg_path_is_uri (char const *path)
{
    char const *p;

    if (path == NULL)
        return FALSE;

    if (strlen (path) < 4)
        return FALSE;

    if ((path[0] < 'a' || path[0] > 'z') &&
        (path[0] < 'A' || path[0] > 'Z')) {
        return FALSE;
    }

    for (p = &path[1];
	    (*p >= 'a' && *p <= 'z') ||
        (*p >= 'A' && *p <= 'Z') ||
        (*p >= '0' && *p <= '9') ||
         *p == '+' ||
         *p == '-' ||
         *p == '.';
        p++);

    if (strlen (p) < 3)
        return FALSE;

    return (p[0] == ':' && p[1] == '/' && p[2] == '/');
}

gchar *
hisvg_get_base_uri_from_filename (const gchar * filename)
{
    gchar *current_dir;
    gchar *absolute_filename;
    gchar *base_uri;


    if (g_path_is_absolute (filename))
        return g_filename_to_uri (filename, NULL, NULL);

    current_dir = g_get_current_dir ();
    absolute_filename = g_build_filename (current_dir, filename, NULL);
    base_uri = g_filename_to_uri (absolute_filename, NULL, NULL);
    g_free (absolute_filename);
    g_free (current_dir);

    return base_uri;
}

/**
 * hisvg_handle_set_base_uri:
 * @handle: A #HiSVGHandle
 * @base_uri: The base uri
 *
 * Set the base URI for this SVG. This can only be called before hisvg_handle_write()
 * has been called.
 *
 * Since: 2.9
 */
void
hisvg_handle_set_base_uri (HiSVGHandle * handle, const char *base_uri)
{
    gchar *uri;
    GFile *file;

    g_return_if_fail (handle != NULL);

    if (base_uri == NULL)
	return;

    if (hisvg_path_is_uri (base_uri)) 
        uri = g_strdup (base_uri);
    else
        uri = hisvg_get_base_uri_from_filename (base_uri);

    file = g_file_new_for_uri (uri ? uri : "data:");
    hisvg_handle_set_base_gfile (handle, file);
    g_object_unref (file);
    g_free (uri);
}

/**
 * hisvg_handle_set_base_gfile:
 * @handle: a #HiSVGHandle
 * @base_file: a #GFile
 *
 * Set the base URI for @handle from @file.
 * Note: This function may only be called before hisvg_handle_write()
 * or hisvg_handle_read_stream_sync() has been called.
 *
 * Since: 2.32
 */
void
hisvg_handle_set_base_gfile (HiSVGHandle *handle,
                            GFile      *base_file)
{
    HiSVGHandlePrivate *priv;

    g_return_if_fail (HISVG_IS_HANDLE (handle));
    g_return_if_fail (G_IS_FILE (base_file));

    priv = handle->priv;

    g_object_ref (base_file);
    if (priv->base_gfile)
        g_object_unref (priv->base_gfile);
    priv->base_gfile = base_file;

    g_free (priv->base_uri);
    priv->base_uri = g_file_get_uri (base_file);
}

/**
 * hisvg_handle_get_base_uri:
 * @handle: A #HiSVGHandle
 *
 * Gets the base uri for this #HiSVGHandle.
 *
 * Returns: the base uri, possibly null
 * Since: 2.8
 */
const char *
hisvg_handle_get_base_uri (HiSVGHandle * handle)
{
    g_return_val_if_fail (handle, NULL);
    return handle->priv->base_uri;
}

/**
 * hisvg_error_quark:
 *
 * The error domain for HISVG
 *
 * Returns: The error domain
 */
GQuark
hisvg_error_quark (void)
{
    /* don't use from_static_string(), since libhisvg might be used in a module
       that's ultimately unloaded */
    return g_quark_from_string ("hisvg-error-quark");
}

static void
hisvg_set_error (GError **error, xmlParserCtxtPtr ctxt)
{
    xmlErrorPtr xerr;

    xerr = xmlCtxtGetLastError (ctxt);
    if (xerr) {
        g_set_error (error, hisvg_error_quark (), 0,
                     _("Error domain %d code %d on line %d column %d of %s: %s"),
                     xerr->domain, xerr->code,
                     xerr->line, xerr->int2,
                     xerr->file ? xerr->file : "data",
                     xerr->message ? xerr->message: "-");
    } else {
        g_set_error (error, hisvg_error_quark (), 0, _("Error parsing XML data"));
    }
}

static gboolean
hisvg_handle_write_impl (HiSVGHandle * handle, const guchar * buf, gsize count, GError ** error)
{
    GError *real_error = NULL;
    int result;

    hisvg_return_val_if_fail (handle != NULL, FALSE, error);

    handle->priv->error = &real_error;
    if (handle->priv->ctxt == NULL) {
        handle->priv->ctxt = xmlCreatePushParserCtxt (&hisvgSAXHandlerStruct, handle, NULL, 0,
                                                      hisvg_handle_get_base_uri (handle));
        _hisvg_set_xml_parse_options(handle->priv->ctxt, handle);

        /* if false, external entities work, but internal ones don't. if true, internal entities
           work, but external ones don't. favor internal entities, in order to not cause a
           regression */
        handle->priv->ctxt->replaceEntities = TRUE;
    }

    result = xmlParseChunk (handle->priv->ctxt, (char *) buf, count, 0);
    if (result != 0) {
        hisvg_set_error (error, handle->priv->ctxt);
        return FALSE;
    }

    handle->priv->error = NULL;

    if (real_error != NULL) {
        g_propagate_error (error, real_error);
        return FALSE;
    }

    return TRUE;
}

static gboolean
hisvg_handle_close_impl (HiSVGHandle * handle, GError ** error)
{
    GError *real_error = NULL;

	handle->priv->is_closed = TRUE;

    handle->priv->error = &real_error;

    if (handle->priv->ctxt != NULL) {
        xmlDocPtr xml_doc;
        int result;

        xml_doc = handle->priv->ctxt->myDoc;

        result = xmlParseChunk (handle->priv->ctxt, "", 0, TRUE);
        if (result != 0) {
            hisvg_set_error (error, handle->priv->ctxt);
            xmlFreeParserCtxt (handle->priv->ctxt);
            xmlFreeDoc (xml_doc);
            return FALSE;
        }

        xmlFreeParserCtxt (handle->priv->ctxt);
        xmlFreeDoc (xml_doc);
    }

    handle->priv->finished = TRUE;
    handle->priv->error = NULL;

    _hisvg_select_css_computed(handle);

    if (real_error != NULL) {
        g_propagate_error (error, real_error);
        return FALSE;
    }

    return TRUE;
}

void
hisvg_drawing_ctx_free (HiSVGDrawingCtx * handle)
{
    hisvg_render_free (handle->render);

    hisvg_state_free_all (handle->state);

	/* the drawsub stack's nodes are owned by the ->defs */
	g_slist_free (handle->drawsub_stack);

    g_warn_if_fail (handle->acquired_nodes == NULL);
    g_slist_free (handle->acquired_nodes);
	
    if (handle->pango_context != NULL)
        g_object_unref (handle->pango_context);

    g_free (handle);
}

void hisvg_handle_get_dimensions (HiSVGHandle* handle, HiSVGDimension* dimension)
{
    if (handle == NULL || dimension == NULL)
    {
        return;
    }

    HiSVGNodeSvg* root = (HiSVGNodeSvg *) handle->priv->treebase;
    if (root == NULL)
    {
        return;
    }
    memset(dimension, 0, sizeof(HiSVGDimension));

    dimension->has_w = root->has_w;
    dimension->has_h = root->has_h;
    dimension->has_vbox = root->has_vbox;

    dimension->w = root->w;
    dimension->h = root->h;
    dimension->vbox.x = root->vbox.rect.x;
    dimension->vbox.y = root->vbox.rect.y;
    dimension->vbox.width = root->vbox.rect.width;
    dimension->vbox.height = root->vbox.rect.height;
}

/**
 * hisvg_handle_get_dimensions_x:
 * @handle: A #HiSVGHandle
 * @dimension_data: (out): A place to store the SVG's size
 *
 * Get the SVG's size. Do not call from within the size_func callback, because an infinite loop will occur.
 *
 * Since: 2.14
 */
void
hisvg_handle_get_dimensions_x (HiSVGHandle * handle, HiSVGDimensionData * dimension_data)
{
    /* This function is probably called from the cairo_render functions.
     * To prevent an infinite loop we are saving the state.
     */
    if (!handle->priv->in_loop) {
        handle->priv->in_loop = TRUE;
        hisvg_handle_get_dimensions_sub (handle, dimension_data, NULL);
        handle->priv->in_loop = FALSE;
    } else {
        /* Called within the size function, so return a standard size */
        dimension_data->em = dimension_data->width = 1;
        dimension_data->ex = dimension_data->height = 1;
    }
}

/**
 * hisvg_handle_get_dimensions_sub:
 * @handle: A #HiSVGHandle
 * @dimension_data: (out): A place to store the SVG's size
 * @id: (nullable): An element's id within the SVG, or %NULL to get
 *   the dimension of the whole SVG.  For example, if you have a layer
 *   called "layer1" for that you want to get the dimension, pass
 *   "#layer1" as the id.
 *
 * Get the size of a subelement of the SVG file. Do not call from within the size_func callback, because an infinite loop will occur.
 *
 * Since: 2.22
 */
gboolean
hisvg_handle_get_dimensions_sub (HiSVGHandle * handle, HiSVGDimensionData * dimension_data, const char *id)
{
    cairo_t *cr;
    cairo_surface_t *target;
    HiSVGDrawingCtx *draw;
    HiSVGNodeSvg *root = NULL;
    HiSVGNode *sself = NULL;
    HiSVGBbox bbox;

    gboolean handle_subelement = TRUE;

    g_return_val_if_fail (handle, FALSE);
    g_return_val_if_fail (dimension_data, FALSE);

    memset (dimension_data, 0, sizeof (HiSVGDimensionData));

    if (id && *id) {
        sself = hisvg_defs_lookup (handle->priv->defs, id);

        if (sself == handle->priv->treebase)
            id = NULL;
    }
    else
        sself = handle->priv->treebase;

    if (!sself && id)
        return FALSE;

    root = (HiSVGNodeSvg *) handle->priv->treebase;

    if (!root)
        return FALSE;

    bbox.rect.x = bbox.rect.y = 0;
    bbox.rect.width = bbox.rect.height = 1;

    if (!id && (root->w.factor == 'p' || root->h.factor == 'p')
            && !root->vbox.active)
        handle_subelement = TRUE;
    else if (!id && root->w.length != -1 && root->h.length != -1)
        handle_subelement = FALSE;

    if (handle_subelement == TRUE) {
        target = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                             1, 1);
        cr = cairo_create  (target);

        draw = hisvg_cairo_new_drawing_ctx (cr, handle);

        if (!draw) {
            cairo_destroy (cr);
            cairo_surface_destroy (target);

            return FALSE;
        }

        while (sself != NULL) {
            draw->drawsub_stack = g_slist_prepend (draw->drawsub_stack, sself);
            sself = HISVG_NODE_PARENT(sself);
        }

        hisvg_state_push (draw);
        cairo_save (cr);

        hisvg_node_draw (handle->priv->treebase, draw, 0);
        bbox = HISVG_CAIRO_RENDER (draw->render)->bbox;

        cairo_restore (cr);
        hisvg_state_pop (draw);
        hisvg_drawing_ctx_free (draw);
        cairo_destroy (cr);
        cairo_surface_destroy (target);

        dimension_data->width = bbox.rect.width;
        dimension_data->height = bbox.rect.height;
    } else {
        bbox.rect.width = root->vbox.rect.width;
        bbox.rect.height = root->vbox.rect.height;

        dimension_data->width = (int) (_hisvg_css_hand_normalize_length (&root->w, handle->priv->dpi_x,
                                       bbox.rect.width + bbox.rect.x * 2, 12) + 0.5);
        dimension_data->height = (int) (_hisvg_css_hand_normalize_length (&root->h, handle->priv->dpi_y,
                                         bbox.rect.height + bbox.rect.y * 2,
                                         12) + 0.5);
    }
    
    dimension_data->em = dimension_data->width;
    dimension_data->ex = dimension_data->height;

    return TRUE;
}

/**
 * hisvg_handle_get_position_sub:
 * @handle: A #HiSVGHandle
 * @position_data: (out): A place to store the SVG fragment's position.
 * @id: An element's id within the SVG.
 * For example, if you have a layer called "layer1" for that you want to get
 * the position, pass "##layer1" as the id.
 *
 * Get the position of a subelement of the SVG file. Do not call from within
 * the size_func callback, because an infinite loop will occur.
 *
 * Since: 2.22
 */
gboolean
hisvg_handle_get_position_sub (HiSVGHandle * handle, HiSVGPositionData * position_data, const char *id)
{
    HiSVGDrawingCtx		*draw;
    HiSVGNodeSvg			*root;
    HiSVGNode			*node;
    HiSVGBbox			 bbox;
    HiSVGDimensionData    dimension_data;
    cairo_surface_t		*target = NULL;
    cairo_t				*cr = NULL;
    gboolean			 ret = FALSE;

    g_return_val_if_fail (handle, FALSE);
    g_return_val_if_fail (position_data, FALSE);

    /* Short-cut when no id is given. */
    if (NULL == id || '\0' == *id) {
        position_data->x = 0;
        position_data->y = 0;
        return TRUE;
    }

    memset (position_data, 0, sizeof (*position_data));
    memset (&dimension_data, 0, sizeof (dimension_data));

    node = hisvg_defs_lookup (handle->priv->defs, id);
    if (!node) {
        return FALSE;
    } else if (node == handle->priv->treebase) {
        /* Root node. */
        position_data->x = 0;
        position_data->y = 0;
        return TRUE;
    }

    root = (HiSVGNodeSvg *) handle->priv->treebase;
    if (!root)
        return FALSE;

    target = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 1, 1);
    cr = cairo_create  (target);
    draw = hisvg_cairo_new_drawing_ctx (cr, handle);
    if (!draw)
        goto bail;

    while (node != NULL) {
        draw->drawsub_stack = g_slist_prepend (draw->drawsub_stack, node);
        node = HISVG_NODE_PARENT(node);
    }

    hisvg_state_push (draw);
    cairo_save (cr);

    hisvg_node_draw (handle->priv->treebase, draw, 0);
    bbox = HISVG_CAIRO_RENDER (draw->render)->bbox;

    cairo_restore (cr);
    hisvg_state_pop (draw);
    hisvg_drawing_ctx_free (draw);

    position_data->x = bbox.rect.x;
    position_data->y = bbox.rect.y;
    dimension_data.width = bbox.rect.width;
    dimension_data.height = bbox.rect.height;

    dimension_data.em = dimension_data.width;
    dimension_data.ex = dimension_data.height;

    ret = TRUE;

bail:
    if (cr)
        cairo_destroy (cr);
    if (target)
        cairo_surface_destroy (target);

    return ret;
}

/** 
 * hisvg_handle_has_sub:
 * @handle: a #HiSVGHandle
 * @id: an element's id within the SVG
 *
 * Checks whether the element @id exists in the SVG document.
 *
 * Returns: %TRUE if @id exists in the SVG document
 *
 * Since: 2.22
 */
gboolean
hisvg_handle_has_sub (HiSVGHandle * handle,
                     const char *id)
{
    g_return_val_if_fail (handle, FALSE);

    if (G_UNLIKELY (!id || !id[0]))
      return FALSE;

    return hisvg_defs_lookup (handle->priv->defs, id) != NULL;
}

/**
 * hisvg_handle_set_dpi:
 * @handle: An #HiSVGHandle
 * @dpi_x: Dots Per Inch (aka Pixels Per Inch)
 * @dpi_y: Dots Per Inch (aka Pixels Per Inch)
 *
 * Sets the DPI for the outgoing pixbuf. Common values are
 * 75, 90, and 300 DPI. Passing a number <= 0 to #dpi_x or @dpi_y will
 * reset the DPI to whatever the default value happens to be.
 *
 * Since: 2.8
 */
void hisvg_handle_set_dpi (HiSVGHandle * handle, double dpi_x, double dpi_y)
{
    g_return_if_fail (handle != NULL);

    if (dpi_x <= 0.)
        handle->priv->dpi_x = hisvg_internal_dpi_x;
    else
        handle->priv->dpi_x = dpi_x;

    if (dpi_y <= 0.)
        handle->priv->dpi_y = hisvg_internal_dpi_y;
    else
        handle->priv->dpi_y = dpi_y;
}

/**
 * hisvg_handle_write:
 * @handle: an #HiSVGHandle
 * @buf: (array length=count) (element-type guchar): pointer to svg data
 * @count: length of the @buf buffer in bytes
 * @error: (allow-none): a location to store a #GError, or %NULL
 *
 * Loads the next @count bytes of the image.  This will return %TRUE if the data
 * was loaded successful, and %FALSE if an error occurred.  In the latter case,
 * the loader will be closed, and will not accept further writes. If %FALSE is
 * returned, @error will be set to an error from the #HiSVGError domain. Errors
 * from #GIOErrorEnum are also possible.
 *
 * Returns: %TRUE on success, or %FALSE on error
 **/
gboolean
hisvg_handle_write (HiSVGHandle * handle, const guchar * buf, gsize count, GError ** error)
{
    HiSVGHandlePrivate *priv;

    hisvg_return_val_if_fail (handle, FALSE, error);
    priv = handle->priv;

    hisvg_return_val_if_fail (!priv->is_closed, FALSE, error);

    if (priv->first_write) {
        priv->first_write = FALSE;

        /* test for GZ marker. todo: store the first 2 bytes in the odd circumstance that someone calls
         * write() in 1 byte increments */
        if ((count >= 2) && (buf[0] == (guchar) 0x1f) && (buf[1] == (guchar) 0x8b)) {
            priv->data_input_stream = g_memory_input_stream_new ();
        }
    }

    if (priv->data_input_stream) {
        g_memory_input_stream_add_data ((GMemoryInputStream *) priv->data_input_stream,
                                        g_memdup (buf, count), count, (GDestroyNotify) g_free);
        return TRUE;
    }

    return hisvg_handle_write_impl (handle, buf, count, error);
}

/**
 * hisvg_handle_close:
 * @handle: a #HiSVGHandle
 * @error: (allow-none): a location to store a #GError, or %NULL
 *
 * Closes @handle, to indicate that loading the image is complete.  This will
 * return %TRUE if the loader closed successfully.  Note that @handle isn't
 * freed until @g_object_unref is called.
 *
 * Returns: %TRUE on success, or %FALSE on error
 **/
gboolean
hisvg_handle_close (HiSVGHandle * handle, GError ** error)
{
    HiSVGHandlePrivate *priv;

    hisvg_return_val_if_fail (handle, FALSE, error);
    priv = handle->priv;

    if (priv->is_closed)
          return TRUE;

    if (priv->data_input_stream) {
        gboolean ret;

        ret = hisvg_handle_read_stream_sync (handle, priv->data_input_stream, NULL, error);
        g_object_unref (priv->data_input_stream);
        priv->data_input_stream = NULL;

        return ret;
    }

    return hisvg_handle_close_impl (handle, error);
}

/**
 * hisvg_handle_read_stream_sync:
 * @handle: a #HiSVGHandle
 * @stream: a #GInputStream
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: (allow-none): a location to store a #GError, or %NULL
 *
 * Reads @stream and writes the data from it to @handle.
 *
 * If @cancellable is not %NULL, then the operation can be cancelled by
 * triggering the cancellable object from another thread. If the
 * operation was cancelled, the error %G_IO_ERROR_CANCELLED will be
 * returned.
 *
 * Returns: %TRUE if reading @stream succeeded, or %FALSE otherwise
 *   with @error filled in
 *
 * Since: 2.32
 */
gboolean
hisvg_handle_read_stream_sync (HiSVGHandle   *handle,
                              GInputStream *stream,
                              GCancellable *cancellable,
                              GError      **error)
{
    HiSVGHandlePrivate *priv;
    xmlParserInputBufferPtr buffer;
    xmlParserInputPtr input;
    int result;
    xmlDocPtr doc;
    GError *err = NULL;
    gboolean res = FALSE;
    const guchar *buf;

    g_return_val_if_fail (HISVG_IS_HANDLE (handle), FALSE);
    g_return_val_if_fail (G_IS_INPUT_STREAM (stream), FALSE);
    g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    priv = handle->priv;

    /* detect zipped streams */
    stream = g_buffered_input_stream_new (stream);
    if (g_buffered_input_stream_fill (G_BUFFERED_INPUT_STREAM (stream), 2, cancellable, error) != 2) {
        g_object_unref (stream);
        return FALSE;
    }
    buf = g_buffered_input_stream_peek_buffer (G_BUFFERED_INPUT_STREAM (stream), NULL);
    if ((buf[0] == 0x1f) && (buf[1] == 0x8b)) {
        GConverter *converter;
        GInputStream *conv_stream;

        converter = G_CONVERTER (g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_GZIP));
        conv_stream = g_converter_input_stream_new (stream, converter);
        g_object_unref (converter);
        g_object_unref (stream);

        stream = conv_stream;
    }

    priv->error = &err;
    priv->cancellable = cancellable ? g_object_ref (cancellable) : NULL;
    if (priv->ctxt == NULL) {
        priv->ctxt = xmlCreatePushParserCtxt (&hisvgSAXHandlerStruct, handle, NULL, 0,
                                              hisvg_handle_get_base_uri (handle));
        _hisvg_set_xml_parse_options(priv->ctxt, handle);

        /* if false, external entities work, but internal ones don't. if true, internal entities
           work, but external ones don't. favor internal entities, in order to not cause a
           regression */
        /* FIXMEchpe: FIX THIS! */
        priv->ctxt->replaceEntities = TRUE;
    }

    buffer = _hisvg_xml_input_buffer_new_from_stream (stream, cancellable, XML_CHAR_ENCODING_NONE, &err);
    input = xmlNewIOInputStream (priv->ctxt, buffer, XML_CHAR_ENCODING_NONE);

    if (xmlPushInput (priv->ctxt, input) < 0) {
        hisvg_set_error (error, priv->ctxt);
        xmlFreeInputStream (input);
        goto out;
    }

    result = xmlParseDocument (priv->ctxt);
    if (result != 0) {
        if (err)
            g_propagate_error (error, err);
        else
            hisvg_set_error (error, handle->priv->ctxt);

        goto out;
    }

    if (err != NULL) {
        g_propagate_error (error, err);
        goto out;
    }

    doc = priv->ctxt->myDoc;
    xmlFreeParserCtxt (priv->ctxt);
    priv->ctxt = NULL;

    xmlFreeDoc (doc);

    priv->finished = TRUE;
    _hisvg_select_css_computed(handle);

    res = TRUE;

  out:

    g_object_unref (stream);

    priv->error = NULL;
    g_clear_object (&priv->cancellable);

    return res;
}

/**
 * hisvg_handle_new_from_gfile_sync:
 * @file: a #GFile
 * @flags: flags from #HiSVGHandleFlags
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: (allow-none): a location to store a #GError, or %NULL
 *
 * Creates a new #HiSVGHandle for @file.
 *
 * If @cancellable is not %NULL, then the operation can be cancelled by
 * triggering the cancellable object from another thread. If the
 * operation was cancelled, the error %G_IO_ERROR_CANCELLED will be
 * returned.
 *
 * Returns: a new #HiSVGHandle on success, or %NULL with @error filled in
 *
 * Since: 2.32
 */
HiSVGHandle *
hisvg_handle_new_from_gfile_sync (GFile          *file,
                                 HiSVGHandleFlags flags,
                                 GCancellable   *cancellable,
                                 GError        **error)
{
    HiSVGHandle *handle;
    GFileInputStream *stream;

    g_return_val_if_fail (G_IS_FILE (file), NULL);
    g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    stream = g_file_read (file, cancellable, error);
    if (stream == NULL)
        return NULL;

    handle = hisvg_handle_new_from_stream_sync (G_INPUT_STREAM (stream), file,
                                               flags, cancellable, error);
    g_object_unref (stream);

    return handle;
}

/**
 * hisvg_handle_new_from_stream_sync:
 * @input_stream: a #GInputStream
 * @base_file: (allow-none): a #GFile, or %NULL
 * @flags: flags from #HiSVGHandleFlags
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: (allow-none): a location to store a #GError, or %NULL
 *
 * Creates a new #HiSVGHandle for @stream.
 *
 * If @cancellable is not %NULL, then the operation can be cancelled by
 * triggering the cancellable object from another thread. If the
 * operation was cancelled, the error %G_IO_ERROR_CANCELLED will be
 * returned.
 *
 * Returns: a new #HiSVGHandle on success, or %NULL with @error filled in
 *
 * Since: 2.32
 */
HiSVGHandle *
hisvg_handle_new_from_stream_sync (GInputStream   *input_stream,
                                  GFile          *base_file,
                                  HiSVGHandleFlags flags,
                                  GCancellable    *cancellable,
                                  GError         **error)
{
    HiSVGHandle *handle;

    g_return_val_if_fail (G_IS_INPUT_STREAM (input_stream), NULL);
    g_return_val_if_fail (base_file == NULL || G_IS_FILE (base_file), NULL);
    g_return_val_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable), NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    handle = hisvg_handle_new (flags);

    if (base_file)
        hisvg_handle_set_base_gfile (handle, base_file);

    if (!hisvg_handle_read_stream_sync (handle, input_stream, cancellable, error)) {
        g_object_unref (handle);
        return NULL;
    }

    return handle;
}

/**
 * hisvg_cleanup:
 *
 * This function should not be called from normal programs.
 * See xmlCleanupParser() for more information.
 *
 * Since: 2.36
 **/
void
hisvg_cleanup (void)
{
    xmlCleanupParser ();
}

void
hisvg_node_set_atts (HiSVGNode * node, HiSVGHandle * ctx, HiSVGPropertyBag * atts)
{
    node->set_atts (node, ctx, atts);
}

void
hisvg_pop_discrete_layer (HiSVGDrawingCtx * ctx)
{
    ctx->render->pop_discrete_layer (ctx);
}

void
hisvg_push_discrete_layer (HiSVGDrawingCtx * ctx)
{
    ctx->render->push_discrete_layer (ctx);
}

/*
 * hisvg_acquire_node:
 * @ctx: The drawing context in use
 * @url: The IRI to lookup
 *
 * Use this function when looking up urls to other nodes. This
 * function does proper recursion checking and thereby avoids
 * infinite loops.
 *
 * Nodes acquired by this function must be released using
 * hisvg_release_node() in reverse acquiring order.
 *
 * Returns: The node referenced by @url or %NULL if the @url
 *          does not reference a node.
 */
HiSVGNode *
hisvg_acquire_node (HiSVGDrawingCtx * ctx, const char *url)
{
  HiSVGNode *node;

  node = hisvg_defs_lookup (ctx->defs, url);
  if (node == NULL)
    return NULL;

  if (g_slist_find (ctx->acquired_nodes, node))
    return NULL;

  ctx->acquired_nodes = g_slist_prepend (ctx->acquired_nodes, node);

  return node;
}

/*
 * hisvg_release_node:
 * @ctx: The drawing context the node was acquired from
 * @node: Node to release
 *
 * Releases a node previously acquired via hisvg_acquire_node().
 *
 * if @node is %NULL, this function does nothing.
 */
void
hisvg_release_node (HiSVGDrawingCtx * ctx, HiSVGNode *node)
{
  if (node == NULL)
    return;

  g_return_if_fail (ctx->acquired_nodes != NULL);
  g_return_if_fail (ctx->acquired_nodes->data == node);

  ctx->acquired_nodes = g_slist_remove (ctx->acquired_nodes, node);
}

void
hisvg_render_path (HiSVGDrawingCtx * ctx, const cairo_path_t *path)
{
    ctx->render->render_path (ctx, path);
    hisvg_render_markers (ctx, path);
}

void
hisvg_render_surface (HiSVGDrawingCtx * ctx, cairo_surface_t *surface, double x, double y, double w, double h)
{
    /* surface must be a cairo image surface */
    g_return_if_fail (cairo_surface_get_type (surface) == CAIRO_SURFACE_TYPE_IMAGE);

    ctx->render->render_surface (ctx, surface, x, y, w, h);
}

void
hisvg_add_clipping_rect (HiSVGDrawingCtx * ctx, double x, double y, double w, double h)
{
    ctx->render->add_clipping_rect (ctx, x, y, w, h);
}

cairo_surface_t *
hisvg_get_surface_of_node (HiSVGDrawingCtx * ctx, HiSVGNode * drawable, double w, double h)
{
    return ctx->render->get_surface_of_node (ctx, drawable, w, h);
}

void
hisvg_render_free (HiSVGRender * render)
{
    render->free (render);
}

void
hisvg_bbox_init (HiSVGBbox * self, cairo_matrix_t *affine)
{
    self->virgin = 1;
    self->affine = *affine;
}

void
hisvg_bbox_insert (HiSVGBbox * dst, HiSVGBbox * src)
{
    cairo_matrix_t affine;
    double xmin, ymin;
    double xmax, ymax;
    int i;

    if (src->virgin)
        return;

    if (!dst->virgin) {
        xmin = dst->rect.x, ymin = dst->rect.y;
        xmax = dst->rect.x + dst->rect.width, ymax = dst->rect.y + dst->rect.height;
    } else {
        xmin = ymin = xmax = ymax = 0;
    }

    affine = dst->affine;
    if (cairo_matrix_invert (&affine) != CAIRO_STATUS_SUCCESS)
      return; //FIXMEchpe correct??

    cairo_matrix_multiply (&affine, &src->affine, &affine);

    for (i = 0; i < 4; i++) {
        double rx, ry, x, y;
        rx = src->rect.x + src->rect.width * (double) (i % 2);
        ry = src->rect.y + src->rect.height * (double) (i / 2);
        x = affine.xx * rx + affine.xy * ry + affine.x0;
        y = affine.yx * rx + affine.yy * ry + affine.y0;
        if (dst->virgin) {
            xmin = xmax = x;
            ymin = ymax = y;
            dst->virgin = 0;
        } else {
            if (x < xmin)
                xmin = x;
            if (x > xmax)
                xmax = x;
            if (y < ymin)
                ymin = y;
            if (y > ymax)
                ymax = y;
        }
    }
    dst->rect.x = xmin;
    dst->rect.y = ymin;
    dst->rect.width = xmax - xmin;
    dst->rect.height = ymax - ymin;
}

void
hisvg_bbox_clip (HiSVGBbox * dst, HiSVGBbox * src)
{
    cairo_matrix_t affine;
	double xmin, ymin;
	double xmax, ymax;
    int i;

    if (src->virgin)
        return;

	if (!dst->virgin) {
        xmin = dst->rect.x + dst->rect.width, ymin = dst->rect.y + dst->rect.height;
        xmax = dst->rect.x, ymax = dst->rect.y;
    } else {
        xmin = ymin = xmax = ymax = 0;
    }

    affine = dst->affine;
    if (cairo_matrix_invert (&affine) != CAIRO_STATUS_SUCCESS)
      return;

    cairo_matrix_multiply (&affine, &src->affine, &affine);

    for (i = 0; i < 4; i++) {
        double rx, ry, x, y;
        rx = src->rect.x + src->rect.width * (double) (i % 2);
        ry = src->rect.y + src->rect.height * (double) (i / 2);
        x = affine.xx * rx + affine.xy * ry + affine.x0;
        y = affine.yx * rx + affine.yy * ry + affine.y0;
        if (dst->virgin) {
            xmin = xmax = x;
            ymin = ymax = y;
            dst->virgin = 0;
        } else {
            if (x < xmin)
                xmin = x;
            if (x > xmax)
                xmax = x;
            if (y < ymin)
                ymin = y;
            if (y > ymax)
                ymax = y;
        }
    }

    if (xmin < dst->rect.x)
        xmin = dst->rect.x;
    if (ymin < dst->rect.y)
        ymin = dst->rect.y;
    if (xmax > dst->rect.x + dst->rect.width)
        xmax = dst->rect.x + dst->rect.width;
    if (ymax > dst->rect.y + dst->rect.height)
        ymax = dst->rect.y + dst->rect.height;

    dst->rect.x = xmin;
    dst->rect.width = xmax - xmin;
    dst->rect.y = ymin;
    dst->rect.height = ymax - ymin;
}

void
_hisvg_push_view_box (HiSVGDrawingCtx * ctx, double w, double h)
{
    HiSVGViewBox *vb = g_new (HiSVGViewBox, 1);
    *vb = ctx->vb;
    ctx->vb_stack = g_slist_prepend (ctx->vb_stack, vb);
    ctx->vb.rect.width = w;
    ctx->vb.rect.height = h;
}

void
_hisvg_pop_view_box (HiSVGDrawingCtx * ctx)
{
    ctx->vb = *((HiSVGViewBox *) ctx->vb_stack->data);
    g_free (ctx->vb_stack->data);
    ctx->vb_stack = g_slist_delete_link (ctx->vb_stack, ctx->vb_stack);
}

void
hisvg_return_if_fail_warning (const char *pretty_function, const char *expression, GError ** error)
{
    g_set_error (error, HISVG_ERROR, 0, _("%s: assertion `%s' failed"), pretty_function, expression);
}

static gboolean
_hisvg_handle_allow_load (HiSVGHandle *handle,
                         const char *uri,
                         GError **error)
{
    HiSVGHandlePrivate *priv = handle->priv;
    GFile *base;
    char *path, *dir;
    char *scheme = NULL, *cpath = NULL, *cdir = NULL;

    g_assert (handle->priv->load_policy == HISVG_LOAD_POLICY_STRICT);

    scheme = g_uri_parse_scheme (uri);

    /* Not a valid URI */
    if (scheme == NULL)
        goto deny;

    /* Allow loads of data: from any location */
    if (g_str_equal (scheme, "data"))
        goto allow;

    /* No base to compare to? */
    if (priv->base_gfile == NULL)
        goto deny;

    /* Deny loads from differing URI schemes */
    if (!g_file_has_uri_scheme (priv->base_gfile, scheme))
        goto deny;

    /* resource: is allowed to load anything from other resources */
    if (g_str_equal (scheme, "resource"))
        goto allow;

    /* Non-file: isn't allowed to load anything */
    if (!g_str_equal (scheme, "file"))
        goto deny;

    base = g_file_get_parent (priv->base_gfile);
    if (base == NULL)
        goto deny;

    dir = g_file_get_path (base);
    g_object_unref (base);

    cdir = realpath (dir, NULL);
    g_free (dir);
    if (cdir == NULL)
        goto deny;

    path = g_filename_from_uri (uri, NULL, NULL);
    if (path == NULL)
        goto deny;

    cpath = realpath (path, NULL);
    g_free (path);

    if (cpath == NULL)
        goto deny;

    /* Now check that @cpath is below @cdir */
    if (!g_str_has_prefix (cpath, cdir) ||
        cpath[strlen (cdir)] != G_DIR_SEPARATOR)
        goto deny;

    /* Allow load! */

 allow:
    g_free (scheme);
    free (cpath);
    free (cdir);
    return TRUE;

 deny:
    g_free (scheme);
    free (cpath);
    free (cdir);

    g_set_error (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                 "File may not link to URI \"%s\"", uri);
    return FALSE;
}

static char *
_hisvg_handle_resolve_uri (HiSVGHandle *handle,
                          const char *uri)
{
    HiSVGHandlePrivate *priv = handle->priv;
    char *scheme, *resolved_uri;
    GFile *base, *resolved;

    if (uri == NULL)
        return NULL;

    scheme = g_uri_parse_scheme (uri);
    if (scheme != NULL ||
        priv->base_gfile == NULL ||
        (base = g_file_get_parent (priv->base_gfile)) == NULL) {
        g_free (scheme);
        return g_strdup (uri);
    }

    resolved = g_file_resolve_relative_path (base, uri);
    resolved_uri = g_file_get_uri (resolved);

    g_free (scheme);
    g_object_unref (base);
    g_object_unref (resolved);

    return resolved_uri;
}

char * 
_hisvg_handle_acquire_data (HiSVGHandle *handle,
                           const char *url,
                           char **content_type,
                           gsize *len,
                           GError **error)
{
    char *uri;
    char *data;

    uri = _hisvg_handle_resolve_uri (handle, url);

    if (_hisvg_handle_allow_load (handle, uri, error)) {
        data = _hisvg_io_acquire_data (uri, 
                                      hisvg_handle_get_base_uri (handle), 
                                      content_type, 
                                      len, 
                                      handle->priv->cancellable,
                                      error);
    } else {
        data = NULL;
    }

    g_free (uri);
    return data;
}

GInputStream *
_hisvg_handle_acquire_stream (HiSVGHandle *handle,
                             const char *url,
                             char **content_type,
                             GError **error)
{
    char *uri;
    GInputStream *stream;

    uri = _hisvg_handle_resolve_uri (handle, url);

    if (_hisvg_handle_allow_load (handle, uri, error)) {
        stream = _hisvg_io_acquire_stream (uri, 
                                          hisvg_handle_get_base_uri (handle), 
                                          content_type, 
                                          handle->priv->cancellable,
                                          error);
    } else {
        stream = NULL;
    }

    g_free (uri);
    return stream;
}

const char* _hisvg_node_get_inner_class_name(HiSVGHandle* handle, HiSVGNode* node)
{
    if (node->inner_class_name == NULL)
    {
        const char* tag =  HISVG_NODE_TAG_NAME(node);
        node->inner_class_name = (char*)calloc(1, strlen(tag) + 16);
        sprintf(node->inner_class_name, "%s_inner_%d", tag, handle->priv->inner_class_name_idx++);
    }
    return node->inner_class_name;
}

gboolean _hisvg_node_attribute_to_node_inner_css_properties(HiSVGHandle* handle,
        HiSVGNode* node, const char* key, const char* value)
{
    if (node == NULL)
    {
        return FALSE;
    }

    const char* inner_class_name = _hisvg_node_get_inner_class_name(handle, node);

    //"  key:value; "
    size_t last_len = node->inner_class_value ? strlen(node->inner_class_value) : 0;
    size_t len =  last_len + strlen(key) + strlen(value) + 4;
    node->inner_class_value = (char*)realloc(node->inner_class_value, len);
    if (last_len)
    {
        strcat(node->inner_class_value, key);
    }
    else
    {
        strcpy(node->inner_class_value, key);
    }
    strcat(node->inner_class_value, ":");
    strcat(node->inner_class_value, value);
    strcat(node->inner_class_value, "; ");

    // rebuild inner_class
    free(node->inner_class);

    // ".inner_class_name { inner_class_value }\n"
    node->inner_class = (char*)calloc(1, strlen(inner_class_name) + strlen(node->inner_class_value) + 10);
    strcpy(node->inner_class, ".");
    strcat(node->inner_class, inner_class_name);
    strcat(node->inner_class, " { ");
    strcat(node->inner_class, node->inner_class_value);
    strcat(node->inner_class, " } ");

    return TRUE;
}

void build_node_attribute_css(HLDomElementNode* node, void* user_data)
{
    uint8_t** attr_css = (uint8_t**)user_data;

    uint8_t* buf = *attr_css;
    HiSVGNode* svg_node = HISVG_NODE_FROM_DOM_NODE(node);
    if (svg_node->inner_class)
    {
        HISVG_NODE_INCLUDE_CLASS(svg_node, svg_node->inner_class_name);
        size_t buf_len =  buf ? strlen(buf) : 0;
        size_t new_len = buf_len + strlen(svg_node->inner_class) + 1;
        buf = (uint8_t*)realloc(buf, new_len);
        memcpy(buf + buf_len, svg_node->inner_class, strlen(svg_node->inner_class));
        buf[new_len - 1] = 0;
    }
    *attr_css = buf;
}

void _fill_select_css_computed(HLDomElementNode* node, void* user_data)
{
    HLUsedSvgValues* svg_value = hilayout_element_node_get_used_svg_value(node);
    if (svg_value == NULL)
    {
        return;
    }

    HiSVGHandle* handle = (HiSVGHandle*)user_data;
    HiSVGNode*  svgNode = HISVG_NODE_FROM_DOM_NODE(node);
    HiSVGState* state = svgNode->state;

    for (int i = 0; i < HISVG_CSS_PROP_N; i++) {
        css_select_prop_dispatch[i].css_select(handle, svgNode, state, svg_value);
    }
}

void _hisvg_select_css_computed(HiSVGHandle* handle)
{
    HiSVGNodeSvg *root = (HiSVGNodeSvg *) handle->priv->treebase;
    fprintf(stderr, "%s:%d:%s width=%f|height=%f\n", __FILE__, __LINE__, __func__, root->vbox.rect.width, root->vbox.rect.height);

    HLMedia hl_media = {
        .width = root->vbox.rect.width,
        .height = root->vbox.rect.height,
        .dpi = 72,
        .density = 72
    };

    HLDomElementNode* element_root = ((HiSVGNode*)root)->base;
    HLCSS* css = hilayout_css_create();

    uint8_t* attr_css = NULL;
    hilayout_element_node_depth_first_search_tree(element_root, build_node_attribute_css, &attr_css);
    if (attr_css)
    {
        fprintf(stderr, "%s:%d:%s width=%f|height=%f|attr_css=%s|attr_css_len=%ld\n", __FILE__, __LINE__, __func__, root->vbox.rect.width, root->vbox.rect.height, attr_css, strlen(attr_css));
        hilayout_css_append_data(css, attr_css, strlen(attr_css));
    }
    hilayout_css_append_data(css, handle->priv->css_buff, handle->priv->css_buff_len);

    hilayout_do_layout(&hl_media, css, element_root);
    hilayout_element_node_depth_first_search_tree(element_root, _fill_select_css_computed, handle);

    hilayout_css_destroy(css);
    free(attr_css);
}

HLDomElementNode* hisvg_handle_get_node (HiSVGHandle* handle, const char* id)
{
    HiSVGNode* node = hisvg_defs_lookup (handle->priv->defs, id);
    if (node == NULL)
    {
        return NULL;
    }

    return node->base;
}

gboolean hisvg_handle_set_stylesheet (HiSVGHandle* handle, const char* id, const guint8* css, gsize css_len, GError** error)
{
    if (handle == NULL || css == NULL || css_len == 0)
    {
        return FALSE;
    }

    if (id == NULL)
    {
        hisvg_parse_cssbuffer (handle, css, css_len);
    }
    else
    {
        HLDomElementNode* element = hisvg_handle_get_node (handle, id);
        if (element == NULL)
        {
            return FALSE;
        }

        char* buf = (char*)calloc(1, css_len + 8);
        strcpy(buf, "#");
        strcat(buf, id);
        strcat(buf, " { ");
        strcat(buf, css);
        strcat(buf, " }");
        hisvg_parse_cssbuffer (handle, buf, strlen(buf));
        free(buf);
    }
    _hisvg_select_css_computed(handle);
    return TRUE;
}
