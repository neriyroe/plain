/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

/* For use within the translation unit. */
MOCOSEL_WORD_DOUBLE MOCOSEL_EXPAND(MOCOSEL_CONTEXT* context, MOCOSEL_LOOKUP function, struct MOCOSEL_LIST* node, struct MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(function != NULL);
    MOCOSEL_ASSERT(node != NULL);
    MOCOSEL_ASSERT(value != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(function == NULL || node == NULL || value == NULL) {
        return 0;
    }
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, function, node, value);
    if(error != 0) {
        return error;
    }
    node = (struct MOCOSEL_LIST*)value->data;
    if(value->type == MOCOSEL_TYPE_LIST) {
        if(node->parent != NULL) {
            error = MOCOSEL_EXPAND(context, function, node, value);
            if(error != 0) {
                return error;
            }
        }
        MOCOSEL_PURGE(node);
    }
    return 0;
}

MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(MOCOSEL_CONTEXT* context, MOCOSEL_LOOKUP function, struct MOCOSEL_LIST* MOCOSEL_RESTRICT node, struct MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(function != NULL);
    MOCOSEL_ASSERT(node != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(function == NULL || node == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(node->keyword.from == NULL) {
        return 0;
    }
    /* Layout. */
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(node);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* argument = MOCOSEL_ARGUMENT(node, index);
        /* Keyword. */
        if(argument->type == MOCOSEL_TYPE_KEYWORD) {
            struct MOCOSEL_VALUE* subvalue = function(context, (const MOCOSEL_GLYPH*)argument->data, node);
            /* MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT. */
            if(subvalue == NULL) {
                return MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT;
            }
            MOCOSEL_RESIZE(value->data, 0, value->length);
            if(subvalue->length > 0) {
                value->data = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(NULL, subvalue->length, 0);
                /* MOCOSEL_ERROR_SYSTEM. */
                if(memcpy(value->data, subvalue->data, subvalue->length) == NULL) {
                    return MOCOSEL_ERROR_SYSTEM;
                }
            } else {
                value->data = subvalue->data;
            }
            value->length = subvalue->length;
            value->type = subvalue->type;
        /* Node. */
        } else if(argument->type == MOCOSEL_TYPE_LIST) {
            if(((struct MOCOSEL_LIST*)argument->data)->parent == NULL) {
                continue;
            }
            MOCOSEL_WORD_DOUBLE error = MOCOSEL_EXPAND(context, function, (struct MOCOSEL_LIST*)argument->data, argument);
            if(error != 0) {
                return error;
            }
            MOCOSEL_PURGE((struct MOCOSEL_LIST*)argument->data);
        }
    }
    struct MOCOSEL_VALUE* subvalue = function(context, NULL, node);
    /* MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT. */
    if(subvalue == NULL) {
        return MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT;
    }
    /* Subroutine. */
    if(subvalue->type == MOCOSEL_TYPE_SUBROUTINE) {
        /* NWW: functions must return values. */
        if(value != NULL) {
            value->data = NULL;
            value->length = 0;
            value->type = MOCOSEL_TYPE_NIL;
        }
        MOCOSEL_WORD_DOUBLE error = ((MOCOSEL_SUBROUTINE)subvalue->data)(context, function, node, value);
        if(error != 0) {
            return error;
        }
    /* Value. */
    } else {
        /* MOCOSEL_ERROR_RUNTIME_WRONG_DATA. */
        if(value == NULL) {
            return MOCOSEL_ERROR_RUNTIME_WRONG_DATA;
        }
        value->data = subvalue->data;
        value->length = subvalue->length;
        value->type = subvalue->type;
    }
    /* Node. */
    if(node->node) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, function, node->node, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
