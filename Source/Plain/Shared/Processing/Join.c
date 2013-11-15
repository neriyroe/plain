/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/24/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_JOIN(MOCOSEL_BYTE* __restrict data, MOCOSEL_WORD_DOUBLE length, struct MOCOSEL_LIST* __restrict node, MOCOSEL_WORD_DOUBLE type) {
    MOCOSEL_ASSERT(node != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = node->layout.to - node->layout.from;
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_RESERVE(sizeof(struct MOCOSEL_VALUE), &node->layout);
    if(error != 0) {
        return error;
    }
    struct MOCOSEL_VALUE* value = (struct MOCOSEL_VALUE*)&node->layout.from[distance];
    switch(type) {
        case MOCOSEL_TYPE_INTEGER:
        case MOCOSEL_TYPE_KEYWORD:
        case MOCOSEL_TYPE_LIST:
        case MOCOSEL_TYPE_REAL:
        case MOCOSEL_TYPE_STRING:
            MOCOSEL_ASSERT(data != NULL);
            MOCOSEL_ASSERT(length > 0);
        case MOCOSEL_TYPE_BOOLEAN:
        case MOCOSEL_TYPE_NIL:
            break;
        
        default:
            return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    value->data = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(NULL, length, 0);
    value->length = length;
    value->type = type;
    if(length > 0) {
        /* Keyword, string. */
        if(type == MOCOSEL_TYPE_KEYWORD || type == MOCOSEL_TYPE_STRING) {
            value->data[--length] = 0;
        }
        /* MOCOSEL_ERROR_SYSTEM. */
        if(memcpy(value->data, data, length) == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        }
    } else {
        value->data = data;
    }
    return 0;
}
