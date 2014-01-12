/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/11/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(const MOCOSEL_GLYPH* MOCOSEL_RESTRICT keyword, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, struct MOCOSEL_VALUE* MOCOSEL_RESTRICT value) {
    MOCOSEL_ASSERT(keyword != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    MOCOSEL_ASSERT(value != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(keyword == NULL || registry == NULL || value == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    struct MOCOSEL_VALUE* destination = MOCOSEL_IMPORT(keyword, registry);
    if(destination == NULL) {
        MOCOSEL_WORD_DOUBLE distance = registry->to - registry->from;
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RESERVE(sizeof(struct MOCOSEL_STATEMENT), registry);
        if(error != 0) {
            return error;
        }
    }
    if(destination->length > 0) {
        MOCOSEL_RESIZE(destination->data, 0, destination->length);
    }
    destination->data = value->data;
    destination->length = value->length;
    destination->type = value->type;
    if(value->length > 0) {
        destination->data = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(NULL, value->length, 0);
        /* MOCOSEL_ERROR_SYSTEM. */
        if(destination->data == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        }
        /* MOCOSEL_ERROR_SYSTEM. */
        if(memcpy(destination->data, value->data, value->length) == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        }
    }
    return 0;
}
