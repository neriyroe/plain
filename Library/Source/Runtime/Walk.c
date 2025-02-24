/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/09/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Plain.h>

PLAIN_WORD_DOUBLE PLAIN_WALK(void* context, PLAIN_SUBROUTINE function, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_ASSERT(function != NULL);
    PLAIN_ASSERT(node != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(function == NULL || node == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    if(node->keyword.from == NULL) {
        return 0;
    }
    /* Arguments. */
    PLAIN_WORD_DOUBLE index = 0;
    PLAIN_WORD_DOUBLE length = PLAIN_ARITY(node);
    for(; index < length; index++) {
        struct PLAIN_VALUE* argument = (struct PLAIN_VALUE*)PLAIN_ARGUMENT(node, index);
        if(argument->type == PLAIN_TYPE_KEYWORD) { /* Variable. */
            struct PLAIN_SEGMENT keyword = {argument->data, argument->data + argument->length};
            if(keyword.from == keyword.to) {
                continue;
            }
            /* Return as it is. */
            PLAIN_WORD_DOUBLE error = function(context, keyword.from, PLAIN_TYPE_KEYWORD, argument);
            if(argument->data != keyword.from) { /* Data has been changed. */
                PLAIN_RESIZE(keyword.from, 0, keyword.to - keyword.from);
            }
            if(error != 0) {
                return error;
            }
        } else if(argument->type == PLAIN_TYPE_LIST) { /* Expression. */
            struct PLAIN_LIST* child = (struct PLAIN_LIST*)argument->data;
            if(child == NULL) {
                continue;
            }
            /* Substitute. */
            PLAIN_WORD_DOUBLE error = PLAIN_WALK(context, function, child, argument);
            if((struct PLAIN_LIST*)argument->data != child) { /* Data has been changed. */
                PLAIN_UNLINK(child);
                PLAIN_RESIZE(child, 0, sizeof(struct PLAIN_LIST));
            }
            if(error != 0) {
                return error;
            }
        }
    }
    PLAIN_WORD_DOUBLE error = function(context, node, PLAIN_TYPE_LIST, value);
    if(error != 0) {
        return error;
    }
    /* Expression. */
    if(node->parent != NULL) {
        if(value->type == PLAIN_TYPE_SUBROUTINE) { /* Subroutine. */
            error = ((PLAIN_SUBROUTINE)value->data)(context, node, PLAIN_TYPE_LIST, value);
            if(error != 0) {
                return error;
            }
        }
    }
    if(node->node != NULL) {
        PLAIN_WORD_DOUBLE error = PLAIN_WALK(context, function, node->node, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
