/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

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
    struct MOCOSEL_VALUE* subvalue = function(context, &node->keyword);
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
