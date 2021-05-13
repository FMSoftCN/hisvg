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
    Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef HISVG_PRIVATE_H
#define HISVG_PRIVATE_H

#include <cairo.h>

#include "hisvg-common.h"

#include <libxml/SAX.h>
#include <libxml/xmlmemory.h>
#include <glib.h>
#include <glib-object.h>
#include <math.h>

#if defined(HAVE_FLOAT_H)
# include <float.h>
#endif

G_BEGIN_DECLS 

typedef struct HiSVGSaxHandler HiSVGSaxHandler;
typedef struct HiSVGDrawingCtx HiSVGDrawingCtx;
typedef struct HiSVGRender HiSVGRender;
typedef GHashTable HiSVGPropertyBag;
typedef struct _HiSVGState HiSVGState;
typedef struct _HiSVGDefs HiSVGDefs;
typedef struct _HiSVGNode HiSVGNode;
typedef struct _HiSVGFilter HiSVGFilter;
typedef struct _HiSVGNodeChars HiSVGNodeChars;

/* prepare for gettext */
#ifndef _
#define _(X) X
#endif

#ifndef N_
#define N_(X) X
#endif

#ifndef M_PI
#  ifdef G_PI
#    define M_PI G_PI
#  else
#    define M_PI 3.14159265358979323846
#  endif                        /* G_PI */
#endif                          /*  M_PI  */

#ifndef DBL_EPSILON
/* 1e-7 is a conservative value.  it's less than 2^(1-24) which is
 * the epsilon value for a 32-bit float.  The regular value for this
 * with 64-bit doubles is 2^(1-53) or approximately 1e-16.
 */
# define DBL_EPSILON 1e-7
#endif

/* HISVG_ONE_MINUS_EPSILON:
 *
 * DBL_EPSILON is the difference between 1 and the least value greater
 * than 1 that is representable in the given floating-point type.  Then
 * 1.0+DBL_EPSILON looks like:
 *
 *         1.00000000000...0000000001 * 2**0
 *
 * while 1.0-DBL_EPSILON looks like:
 *
 *         0.11111111111...1111111111 * 2**0
 *
 * and so represented as:
 *
 *         1.1111111111...11111111110 * 2**-1
 *
 * so, in fact, 1.0-(DBL_EPSILON*.5) works too, but I don't think it
 * really matters.  So, I'll go with the simple 1.0-DBL_EPSILON here.
 *
 * The following python session shows these observations:
 *
 *         >>> 1.0 + 2**(1-53)
 *         1.0000000000000002
 *         >>> 1.0 + 2**(1-54)
 *         1.0
 *         >>> 1.0 - 2**(1-53)
 *         0.99999999999999978
 *         >>> 1.0 - 2**(1-54)
 *         0.99999999999999989
 *         >>> 1.0 - 2**(1-53)*.5
 *         0.99999999999999989
 *         >>> 1.0 - 2**(1-55)
 *         1.0
 */
#define HISVG_ONE_MINUS_EPSILON (1.0 - DBL_EPSILON)

struct HiSVGSaxHandler {
    void (*free) (HiSVGSaxHandler * self);
    void (*start_element) (HiSVGSaxHandler * self, const char *name, HiSVGPropertyBag * atts);
    void (*end_element) (HiSVGSaxHandler * self, const char *name);
    void (*characters) (HiSVGSaxHandler * self, const char *ch, int len);
};

typedef enum {
    HISVG_LOAD_POLICY_STRICT
} HiSVGLoadPolicy;

#define HISVG_LOAD_POLICY_DEFAULT (HISVG_LOAD_POLICY_STRICT)

struct HiSVGHandlePrivate {
    HiSVGHandleFlags flags;

    HiSVGLoadPolicy load_policy;

    gboolean is_disposed;
    gboolean is_closed;

    /* stack; there is a state for each element */

    HiSVGDefs *defs;
    guint nest_level;
    HiSVGNode *currentnode;
    /* this is the root level of the displayable tree, essentially what the
       file is converted into at the end */
    HiSVGNode *treebase;

    GHashTable *css_props;

    /* not a handler stack. each nested handler keeps
     * track of its parent
     */
    HiSVGSaxHandler *handler;
    int handler_nest;

    GHashTable *entities;       /* g_malloc'd string -> xmlEntityPtr */

    xmlParserCtxtPtr ctxt;
    GError **error;
    GCancellable *cancellable;

    double dpi_x;
    double dpi_y;

    // TODO  : clear begin
    GString *title;
    GString *desc;
    GString *metadata;
    // TODO : clear end

    gchar *base_uri;
    GFile *base_gfile;

    gboolean finished;

    gboolean in_loop;		/* see get_dimension() */

    gboolean first_write;
    GInputStream *data_input_stream; /* for hisvg_handle_write of svgz data */

    uint32_t inner_class_name_idx;

    uint8_t* css_buff;
    size_t css_buff_len;
};

typedef struct {
    cairo_rectangle_t rect;
    gboolean active;
} HiSVGViewBox;

/*Contextual information for the drawing phase*/

struct HiSVGDrawingCtx {
    HiSVGRender *render;
    HiSVGState *state;
    GError **error;
    HiSVGDefs *defs;
    void* text_context;
    double dpi_x, dpi_y;
    HiSVGViewBox vb;
    GSList *vb_stack;
    GSList *drawsub_stack;
    GSList *acquired_nodes;
};

/*Abstract base class for context for our backends (one as yet)*/

typedef enum {
  HISVG_RENDER_TYPE_INVALID,

  HISVG_RENDER_TYPE_BASE,

  HISVG_RENDER_TYPE_CAIRO = 8,
  HISVG_RENDER_TYPE_CAIRO_CLIP
} HiSVGRenderType;

struct HiSVGRender {
    HiSVGRenderType type;

    void (*free) (HiSVGRender * self);

    void*            (*create_text_context)     (HiSVGDrawingCtx * ctx, HiSVGState * state);
    void             (*render_text)	            (HiSVGDrawingCtx * ctx, void* layout, double x, double y);
    void             (*render_path)             (HiSVGDrawingCtx * ctx, const cairo_path_t *path);
    void             (*render_surface)          (HiSVGDrawingCtx * ctx, cairo_surface_t *surface,
                                                 double x, double y, double w, double h);
    void             (*pop_discrete_layer)      (HiSVGDrawingCtx * ctx);
    void             (*push_discrete_layer)     (HiSVGDrawingCtx * ctx);
    void             (*add_clipping_rect)       (HiSVGDrawingCtx * ctx, double x, double y,
                                                 double w, double h);
    cairo_surface_t *(*get_surface_of_node)     (HiSVGDrawingCtx * ctx, HiSVGNode * drawable,
                                                 double w, double h);
};

static inline HiSVGRender *
_hisvg_render_check_type (HiSVGRender *render,
                         HiSVGRenderType type)
{
  g_assert ((render->type & type) == type);
  return render;
}

#define _HISVG_RENDER_CIC(render, render_type, RenderCType) \
  ((RenderCType*) _hisvg_render_check_type ((render), (render_type)))

typedef struct {
    cairo_rectangle_t rect;
    cairo_matrix_t affine;
    gboolean virgin;
} HiSVGBbox;

typedef enum {
    objectBoundingBox, userSpaceOnUse
} HiSVGCoordUnits;

typedef enum {
    HISVG_NODE_TYPE_INVALID = 0,

    HISVG_NODE_TYPE_CHARS,
    HISVG_NODE_TYPE_CIRCLE,
    HISVG_NODE_TYPE_CLIP_PATH,
    HISVG_NODE_TYPE_COMPONENT_TRANFER_FUNCTION,
    HISVG_NODE_TYPE_DEFS,
    HISVG_NODE_TYPE_ELLIPSE,
    HISVG_NODE_TYPE_FILTER,
    HISVG_NODE_TYPE_GROUP,
    HISVG_NODE_TYPE_IMAGE,
    HISVG_NODE_TYPE_LIGHT_SOURCE,
    HISVG_NODE_TYPE_LINE,
    HISVG_NODE_TYPE_LINEAR_GRADIENT,
    HISVG_NODE_TYPE_MARKER,
    HISVG_NODE_TYPE_MASK,
    HISVG_NODE_TYPE_PATH,
    HISVG_NODE_TYPE_PATTERN,
    HISVG_NODE_TYPE_POLYGON,
    HISVG_NODE_TYPE_POLYLINE,
    HISVG_NODE_TYPE_RADIAL_GRADIENT,
    HISVG_NODE_TYPE_RECT,
    HISVG_NODE_TYPE_STOP,
    HISVG_NODE_TYPE_SVG,
    HISVG_NODE_TYPE_SWITCH,
    HISVG_NODE_TYPE_SYMBOL,
    HISVG_NODE_TYPE_TEXT,
    HISVG_NODE_TYPE_TREF,
    HISVG_NODE_TYPE_TSPAN,
    HISVG_NODE_TYPE_USE,

    /* Filter primitives */
    HISVG_NODE_TYPE_FILTER_PRIMITIVE = 64,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_BLEND,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_COLOR_MATRIX,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_COMPONENT_TRANSFER,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_COMPOSITE,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_CONVOLVE_MATRIX,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_DIFFUSE_LIGHTING,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_DISPLACEMENT_MAP,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_ERODE,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_FLOOD,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_GAUSSIAN_BLUR,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_IMAGE,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_MERGE,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_MERGE_NODE,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_OFFSET,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_SPECULAR_LIGHTING,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_TILE,
    HISVG_NODE_TYPE_FILTER_PRIMITIVE_TURBULENCE,

} HiSVGNodeType;



struct _HiSVGNode {
    HLDomElementNode* base;
    HiSVGState *state;
    HiSVGNodeType type;
    char* inner_class_name;
    char* inner_class_value;
    char* inner_class;
    void (*free) (HiSVGNode * self);
    void (*draw) (HiSVGNode * self, HiSVGDrawingCtx * ctx, int dominate);
    void (*set_atts) (HiSVGNode * self, HiSVGHandle * ctx, HiSVGPropertyBag *);
};

#define HISVG_NODE_EXTRA_INDEX    0
#define HISVG_NODE_BIND_DOM_NODE(svgNode, domNode) (hilayout_element_node_set_attach_data(domNode, HISVG_NODE_EXTRA_INDEX, svgNode, NULL))
#define HISVG_NODE_FROM_DOM_NODE(domNode) ((HiSVGNode*)hilayout_element_node_get_attach_data(domNode, HISVG_NODE_EXTRA_INDEX))

#define HISVG_NODE_TYPE(node)                ((node)->type)
#define HISVG_NODE_IS_FILTER_PRIMITIVE(node) (HISVG_NODE_TYPE((node)) & HISVG_NODE_TYPE_FILTER_PRIMITIVE)
#define HISVG_NODE_TAG_NAME(node)            (hilayout_element_node_get_tag_name(node->base))
#define HISVG_NODE_HAS_PARENT(node)          (NULL != hilayout_element_node_get_parent(node->base))
#define HISVG_NODE_PARENT(node)              (HISVG_NODE_FROM_DOM_NODE(hilayout_element_node_get_parent(node->base)))
#define HISVG_NODE_ADD_CHILD(node, child)    (hilayout_element_node_append_as_last_child(child->base, node->base))
#define HISVG_NODE_CHILDREN_COUNT(node)      (hilayout_element_node_get_children_count(node->base))
#define HISVG_NODE_SET_STYLE(node, style)    (hilayout_element_node_set_style(node->base, style))
#define HISVG_NODE_GET_STYLE(node)           (hilayout_element_node_get_style(node->base))
#define HISVG_NODE_SET_ID(node, id)    (hilayout_element_node_set_id(((HiSVGNode*)node)->base, id))
#define HISVG_NODE_GET_ID(node)           (hilayout_element_node_get_id(node->base))
#define HISVG_NODE_SET_GENERAL_ATTR(node, key, value) (hilayout_element_node_set_general_attr(((HiSVGNode*)node)->base, key, value))
#define HISVG_NODE_GET_GENERAL_ATTR(node, key, value) (hilayout_element_node_get_general_attr(((HiSVGNode*)node)->base, key))
#define HISVG_NODE_INCLUDE_CLASS(node, klazz)     (hilayout_element_node_include_class(((HiSVGNode*)node)->base, klazz))


#define HISVG_DOM_ELEMENT_NODE_ADD_CHILD(node, child)  (hilayout_element_node_append_as_last_child(child, node))
#define HISVG_DOM_ELEMENT_NODE_PARENT(element)         (hilayout_element_node_get_parent(element))
#define HISVG_DOM_ELEMENT_NODE_FIRST_CHILD(element)    (hilayout_element_node_get_first_child(element))
#define HISVG_DOM_ELEMENT_NODE_LAST_CHILD(element)     (hilayout_element_node_get_last_child(element))
#define HISVG_DOM_ELEMENT_NODE_PREV(element)           (hilayout_element_node_get_prev(element))
#define HISVG_DOM_ELEMENT_NODE_NEXT(element)           (hilayout_element_node_get_next(element))

struct _HiSVGNodeChars {
    HiSVGNode super;
    GString *contents;
};

typedef void (*HiSVGPropertyBagEnumFunc) (const char *key, const char *value, gpointer user_data);

G_GNUC_INTERNAL
HiSVGPropertyBag	    *hisvg_property_bag_new       (const char **atts);
G_GNUC_INTERNAL
HiSVGPropertyBag	    *hisvg_property_bag_dup       (HiSVGPropertyBag * bag);
G_GNUC_INTERNAL
void                 hisvg_property_bag_free      (HiSVGPropertyBag * bag);
G_GNUC_INTERNAL
const char          *hisvg_property_bag_lookup    (HiSVGPropertyBag * bag, const char *key);
G_GNUC_INTERNAL
guint                hisvg_property_bag_size	     (HiSVGPropertyBag * bag);
G_GNUC_INTERNAL
void                 hisvg_property_bag_enumerate (HiSVGPropertyBag * bag, HiSVGPropertyBagEnumFunc func,
                                                  gpointer user_data);

G_GNUC_INTERNAL
gboolean     hisvg_eval_switch_attributes	(HiSVGPropertyBag * atts, gboolean * p_has_cond);
G_GNUC_INTERNAL
gchar       *hisvg_get_base_uri_from_filename    (const gchar * file_name);
G_GNUC_INTERNAL
void hisvg_pop_discrete_layer    (HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
void hisvg_push_discrete_layer   (HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
HiSVGNode *hisvg_acquire_node     (HiSVGDrawingCtx * ctx, const char *url);
G_GNUC_INTERNAL
void hisvg_release_node          (HiSVGDrawingCtx * ctx, HiSVGNode *node);
G_GNUC_INTERNAL
void hisvg_render_path           (HiSVGDrawingCtx * ctx, const cairo_path_t *path);
G_GNUC_INTERNAL
void hisvg_render_surface        (HiSVGDrawingCtx * ctx, cairo_surface_t *surface,
                                 double x, double y, double w, double h);
G_GNUC_INTERNAL
void hisvg_render_free           (HiSVGRender * render);
G_GNUC_INTERNAL
void hisvg_add_clipping_rect     (HiSVGDrawingCtx * ctx, double x, double y, double w, double h);
G_GNUC_INTERNAL
cairo_surface_t *hisvg_get_surface_of_node (HiSVGDrawingCtx * ctx, HiSVGNode * drawable, double w, double h);
G_GNUC_INTERNAL
void hisvg_node_set_atts (HiSVGNode * node, HiSVGHandle * ctx, HiSVGPropertyBag * atts);
G_GNUC_INTERNAL
void hisvg_drawing_ctx_free (HiSVGDrawingCtx * handle);
G_GNUC_INTERNAL
void hisvg_bbox_init     (HiSVGBbox * self, cairo_matrix_t *matrix);
G_GNUC_INTERNAL
void hisvg_bbox_insert   (HiSVGBbox * dst, HiSVGBbox * src);
G_GNUC_INTERNAL
void hisvg_bbox_clip     (HiSVGBbox * dst, HiSVGBbox * src);
G_GNUC_INTERNAL
double _hisvg_css_normalize_length       (const HiSVGLength * in, HiSVGDrawingCtx * ctx, char dir);
G_GNUC_INTERNAL
double _hisvg_css_hand_normalize_length  (const HiSVGLength * in, gdouble pixels_per_inch,
                                         gdouble width_or_height, gdouble font_size);
double _hisvg_css_normalize_font_size    (HiSVGState * state, HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
double _hisvg_css_accumulate_baseline_shift (HiSVGState * state, HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
HiSVGLength _hisvg_css_parse_length (const char *str);
G_GNUC_INTERNAL
void _hisvg_push_view_box    (HiSVGDrawingCtx * ctx, double w, double h);
G_GNUC_INTERNAL
void _hisvg_pop_view_box	    (HiSVGDrawingCtx * ctx);
G_GNUC_INTERNAL
void hisvg_SAX_handler_struct_init (void);
G_GNUC_INTERNAL
char *hisvg_get_url_string (const char *str);
G_GNUC_INTERNAL
void hisvg_return_if_fail_warning (const char *pretty_function,
                                  const char *expression, GError ** error);

G_GNUC_INTERNAL
char *_hisvg_handle_acquire_data (HiSVGHandle *handle,
                                 const char *uri,
                                 char **content_type,
                                 gsize *len,
                                 GError **error);
G_GNUC_INTERNAL
GInputStream *_hisvg_handle_acquire_stream (HiSVGHandle *handle,
                                           const char *uri,
                                           char **content_type,
                                           GError **error);

gboolean _hisvg_node_attribute_to_node_inner_css_properties(HiSVGHandle* handle,
        HiSVGNode* node, const char* key, const char* value);

void _hisvg_select_css_computed(HiSVGHandle* handle);

#define hisvg_return_if_fail(expr, error)    G_STMT_START{			\
     if G_LIKELY(expr) { } else                                     \
       {                                                            \
           hisvg_return_if_fail_warning (G_STRFUNC,                  \
                                        #expr, error);              \
           return;                                                  \
       };				}G_STMT_END

#define hisvg_return_val_if_fail(expr,val,error)	G_STMT_START{       \
     if G_LIKELY(expr) { } else                                     \
       {                                                            \
           hisvg_return_if_fail_warning (G_STRFUNC,                  \
                                        #expr, error);              \
           return (val);                                            \
       };				}G_STMT_END

G_END_DECLS

#endif
