/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 07/20/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(MOCOSEL_CONTEXT* context, MOCOSEL_LOOKUP function, struct MOCOSEL_LIST* node, struct MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(function != NULL);
    MOCOSEL_ASSERT(node != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(function == NULL || node == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(node->keyword.from == NULL) {
        return 0;
    }
    /* Arguments. */
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(node);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* argument = (struct MOCOSEL_VALUE*)MOCOSEL_ARGUMENT(node, index);
        /* Variable. */
        if(argument->type == MOCOSEL_TYPE_KEYWORD) {
            struct MOCOSEL_SEGMENT keyword = {(MOCOSEL_BYTE*)argument->data, (MOCOSEL_BYTE*)argument->data + argument->length};
            struct MOCOSEL_VALUE* subvalue = function(context, &keyword);
            /* MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT. */
            if(subvalue == NULL) {
                return MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT;
            }
            argument->data = subvalue->data;
            argument->length = subvalue->length;
            argument->type = subvalue->type;
            if(argument->data != keyword.from) {
                MOCOSEL_RESIZE(keyword.from, 0, keyword.to - keyword.from);
            }
        /* Expression. */
        } else if(argument->type == MOCOSEL_TYPE_LIST) {
            struct MOCOSEL_LIST* node = (struct MOCOSEL_LIST*)argument->data;
            if(node->parent == NULL) {
                continue;
            }
            MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, function, node, argument);
            if(argument->data != (MOCOSEL_BYTE*)node) {
                MOCOSEL_UNLINK(node);
            }
            if(error != 0) {
                return error;
            }
        }
    }
    struct MOCOSEL_VALUE* subvalue = function(context, &node->keyword);
    /* MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT. */
    if(subvalue == NULL) {
        return MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT;
    }
    /* Subroutine. */
    if(subvalue->type == MOCOSEL_TYPE_SUBROUTINE) {
        MOCOSEL_WORD_DOUBLE error = ((MOCOSEL_SUBROUTINE)subvalue->data)(context, function, node, value);
        if(error != 0) {
            return error;
        }
    /* Value. */
    } else if(value != NULL) {
        value->data = subvalue->data;
        value->length = subvalue->length;
        value->type = subvalue->type;
    }
    /* Node. */
    if(node->node != NULL) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, function, node->node, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
