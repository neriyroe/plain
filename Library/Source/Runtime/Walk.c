/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 11/03/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(void* context, MOCOSEL_SUBROUTINE function, struct MOCOSEL_LIST* node, struct MOCOSEL_VALUE* value) {
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
        if(argument->type == MOCOSEL_TYPE_KEYWORD) { /* Variable. */
            struct MOCOSEL_SEGMENT keyword = {argument->data, argument->data + argument->length};
            if(keyword.from == keyword.to) {
                continue;
            }
            /* Return as it is. */
            MOCOSEL_WORD_DOUBLE error = function(context, keyword.from, MOCOSEL_TYPE_KEYWORD, argument);
            if(argument->data != keyword.from) { /* Data has been changed. */
                MOCOSEL_RESIZE(keyword.from, 0, keyword.to - keyword.from);
            }
            if(error != 0) {
                return error;
            }
        } else if(argument->type == MOCOSEL_TYPE_LIST) { /* Expression. */
            struct MOCOSEL_LIST* child = (struct MOCOSEL_LIST*)argument->data;
            if(child == NULL) {
                continue;
            }
            /* Substitute. */
            MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, function, child, argument);
            if((struct MOCOSEL_LIST*)argument->data != child) { /* Data has been changed. */
                MOCOSEL_UNLINK(child);
                MOCOSEL_RESIZE(child, 0, sizeof(struct MOCOSEL_LIST));
            }
            if(error != 0) {
                return error;
            }
        }
    }
    if(value == NULL) {
       value = (struct MOCOSEL_VALUE*)MOCOSEL_AUTO(sizeof(struct MOCOSEL_VALUE));
    }
    MOCOSEL_WORD_DOUBLE error = function(context, node, MOCOSEL_TYPE_LIST, value);
    if(error != 0) {
        return error;
    }
    if(value->data != NULL) {
        /* Subroutine. */
        if(value->type == MOCOSEL_TYPE_SUBROUTINE) {
            error = ((MOCOSEL_SUBROUTINE)value->data)(context, node, MOCOSEL_TYPE_LIST, value);
            if(error != 0) {
                return error;
            }
        }
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
