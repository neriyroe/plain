/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/11/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_YIELD(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_SEGMENT* __restrict set) {
    MOCOSEL_ASSERT(node != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    MOCOSEL_ASSERT(set != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL || registry == NULL || set == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_BYTE* from = node->layout.from;
    MOCOSEL_BYTE* to = node->layout.to;
    for(; from != to; from += sizeof(struct MOCOSEL_VALUE)) {
        struct MOCOSEL_VALUE* source = (struct MOCOSEL_VALUE*)from;
        if(source->type == MOCOSEL_TYPE_LIST) {
            struct MOCOSEL_LIST* list = (struct MOCOSEL_LIST*)source->data;
            if(list->parent != NULL) {
                MOCOSEL_NONE(value);
                /* Walk. */
                MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(list, registry, &value);
                if(error != 0) {
                    return error;
                }
                /* Nested. */
                if(value.type == MOCOSEL_TYPE_LIST) {
                    error = MOCOSEL_YIELD((struct MOCOSEL_LIST*)value.data, registry, set);
                    if(error != 0) {
                        return error;
                    }
                /* Primitive. */
                } else {
                    MOCOSEL_WORD_DOUBLE distance = set->to - set->from;
                    MOCOSEL_WORD_DOUBLE number = distance + value.length + sizeof(struct MOCOSEL_VALUE);
                    set->from = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(set->from, number, distance);
                    set->to = set->from + number;
                    /* MOCOSEL_ERROR_SYSTEM. */
                    if(set->from == NULL) {
                        return MOCOSEL_ERROR_SYSTEM;
                    }
                    struct MOCOSEL_VALUE* destination = (struct MOCOSEL_VALUE*)memcpy(set->from + distance, &value, sizeof(struct MOCOSEL_VALUE));
                    /* Contiguous. */
                    if(value.length > 0) {
                        destination->data = (MOCOSEL_BYTE*)memcpy(set->from + distance + sizeof(struct MOCOSEL_VALUE), value.data, value.length);
                    }
                }
            }
        }
    }
    return 0;
}
