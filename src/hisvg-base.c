
#include "hisvg.h"
#include "hisvg-base.h"
#include <glib.h>

static xmlSAXHandler _hisvgSAXHandlerStruct;

void _hisvg_set_load_flags(HiSVGLoadFlags* load_flags, HiSVGHandleFlags flags)
{
    if (load_flags == NULL)
    {
        return;
    }
    load_flags->unlimited_size = flags & HISVG_HANDLE_FLAG_UNLIMITED;
    load_flags->keep_image_data = flags & HISVG_HANDLE_FLAG_KEEP_IMAGE_DATA;
}

HiSVGHandleFlags _hisvg_get_load_flags(HiSVGLoadFlags* load_flags)
{
    if (load_flags == NULL)
    {
        return HISVG_HANDLE_FLAGS_NONE;
    }

    return (load_flags->unlimited_size << 0) | (load_flags->keep_image_data << 1);
}

void _hisvg_set_testing(HiSVGHandle* handle, uint8_t testing)
{
    if (handle)
    {
        handle->is_testing = testing;
    }
}

uint8_t _hisvg_get_testing(HiSVGHandle* handle)
{
    return handle ? handle->is_testing : 0;
}

void _hisvg_return_if_fail_warning (const char* pretty_function, const char* expression, GError** error)
{
    g_set_error (error, HISVG_ERROR, 0, "%s: assertion `%s' failed", pretty_function, expression);
}

void _hisvg_set_xml_parse_options(xmlParserCtxtPtr xml_parser, HiSVGHandle* ctx)
{
    xml_parser->options |= XML_PARSE_NONET;

    if (ctx->load_flags.unlimited_size) {
#if LIBXML_VERSION > 20632
        xml_parser->options |= XML_PARSE_HUGE;
#endif
    }

#if LIBXML_VERSION > 20800
    xml_parser->options |= XML_PARSE_BIG_LINES;
#endif
}

void _hisvg_set_error (GError** error, xmlParserCtxtPtr ctxt)
{
    xmlErrorPtr xerr;

    xerr = xmlCtxtGetLastError (ctxt);
    if (xerr) {
        g_set_error (error, hisvg_error_quark (), 0,
                     "Error domain %d code %d on line %d column %d of %s: %s",
                     xerr->domain, xerr->code,
                     xerr->line, xerr->int2,
                     xerr->file ? xerr->file : "data",
                     xerr->message ? xerr->message: "-");
    } else {
        g_set_error (error, hisvg_error_quark (), 0, "Error parsing XML data");
    }
}

gboolean  _hisvg_handle_write (HiSVGHandle* handle, const guchar* buf,
                                     gsize count, GError** error)
{
    GError* real_error = NULL;
    int result;

    _hisvg_return_val_if_fail (handle != NULL, FALSE, error);

    handle->error = &real_error;
    if (handle->ctxt == NULL) {
        handle->ctxt = xmlCreatePushParserCtxt (&_hisvgSAXHandlerStruct, handle, NULL, 0,
                                                      hisvg_handle_get_base_uri (handle));
        _hisvg_set_xml_parse_options(handle->ctxt, handle);

        /* if false, external entities work, but internal ones don't. if true, internal entities
           work, but external ones don't. favor internal entities, in order to not cause a
           regression */
        handle->ctxt->replaceEntities = TRUE;
    }

    result = xmlParseChunk (handle->ctxt, (char *) buf, count, 0);
    if (result != 0) {
        _hisvg_set_error (error, handle->ctxt);
        return FALSE;
    }

    handle->error = NULL;

    if (real_error != NULL) {
        g_propagate_error (error, real_error);
        return FALSE;
    }

    return TRUE;
}

gboolean  _hisvg_handle_close (HiSVGHandle* handle, GError** error)
{
    GError *real_error = NULL;

	handle->is_closed = TRUE;

    handle->error = &real_error;

    if (handle->ctxt != NULL) {
        xmlDocPtr xml_doc;
        int result;

        xml_doc = handle->ctxt->myDoc;

        result = xmlParseChunk (handle->ctxt, "", 0, TRUE);
        if (result != 0) {
            _hisvg_set_error (error, handle->ctxt);
            xmlFreeParserCtxt (handle->ctxt);
            xmlFreeDoc (xml_doc);
            return FALSE;
        }

        xmlFreeParserCtxt (handle->ctxt);
        xmlFreeDoc (xml_doc);
    }

    handle->finished = TRUE;
    handle->error = NULL;

    if (real_error != NULL) {
        g_propagate_error (error, real_error);
        return FALSE;
    }

    return TRUE;
}

gboolean hisvg_handle_fill_with_data (HiSVGHandle* handle, const char* data, 
        gsize data_len, GError** error)
{
    gboolean rv = FALSE;
    if (handle == NULL || data == NULL || data_len == 0)
    {
        return rv;
    }

    rv = _hisvg_handle_write (handle, (guchar *) data, data_len, error);
    return _hisvg_handle_close (handle, rv ? error : NULL) && rv;
}
