/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/24/2013,
 * Revision 10/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

/* Safe, contiguous. */
MOCOSEL_INLINE MOCOSEL_WORD_DOUBLE MOCOSEL_JOIN_DATA(MOCOSEL_BYTE* __restrict data, MOCOSEL_WORD_DOUBLE length, int type, struct MOCOSEL_VALUE* __restrict value) {
    MOCOSEL_ASSERT(data != NULL);
    MOCOSEL_ASSERT(length > 0);
    MOCOSEL_ASSERT(value != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(data == NULL || length < 1 || value == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    value->data = (MOCOSEL_BYTE*)value + sizeof(struct MOCOSEL_VALUE);
    value->stride = length;
    value->type = type;
    /* MOCOSEL_ERROR_SYSTEM. */
    if(memcpy(value->data, data, length) == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    return 0;
}

/* Unsafe, weak. */
MOCOSEL_INLINE MOCOSEL_WORD_DOUBLE MOCOSEL_JOIN_DATUM(MOCOSEL_BYTE* __restrict data, MOCOSEL_WORD_DOUBLE length, int type, struct MOCOSEL_VALUE* __restrict value) {
    MOCOSEL_ASSERT(value != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(value == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    value->data = data;
    value->stride = length;
    value->type = type;
    return 0;
}

MOCOSEL_WORD_DOUBLE MOCOSEL_JOIN(MOCOSEL_BYTE* __restrict data, MOCOSEL_WORD_DOUBLE length, struct MOCOSEL_LIST* __restrict node, MOCOSEL_WORD_DOUBLE type) {
    MOCOSEL_ASSERT(node != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = node->layout.to - node->layout.from;
    MOCOSEL_WORD_DOUBLE number = distance + length + sizeof(struct MOCOSEL_VALUE);
    node->layout.from = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(node->layout.from, number, distance);
    node->layout.to = node->layout.from + number;
    /* MOCOSEL_ERROR_SYSTEM. */
    if(node->layout.from == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    struct MOCOSEL_VALUE* value = (struct MOCOSEL_VALUE*)&node->layout.from[distance];
    /* Boolean, nil. */
    if(type == MOCOSEL_TYPE_BOOLEAN || type == MOCOSEL_TYPE_NIL) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN_DATUM(data, length, type, value);
        if(error != 0) {
            return error;
        }
    /* Integer, real. */
    } else if(type == MOCOSEL_TYPE_INTEGER || type == MOCOSEL_TYPE_REAL) {
        MOCOSEL_ASSERT(data != NULL);
        MOCOSEL_ASSERT(length > 0);
        /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
        if(data == NULL || length < 1) {
            return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN_DATA(data, length, type, value);
        if(error != 0) {
            return error;
        }
    /* Keyword, string. */
    } else if(type == MOCOSEL_TYPE_KEYWORD || type == MOCOSEL_TYPE_STRING) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN_DATA(data, --length, type, value);
        if(error != 0) {
            return error;
        }
        value->data[value->stride++] = 0;
    /* List. */
    } else if(type == MOCOSEL_TYPE_LIST) {
        MOCOSEL_ASSERT(data != NULL);
        MOCOSEL_ASSERT(length >= sizeof(struct MOCOSEL_LIST));
        /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
        if(data == NULL || length < 1) {
            return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN_DATA(data, length, type, value);
        if(error != 0) {
            return error;
        }
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    } else {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    return 0;
}
