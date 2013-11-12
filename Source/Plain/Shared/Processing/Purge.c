/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/08/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

void MOCOSEL_PURGE(struct MOCOSEL_LIST* node) {
    if(node == NULL) {
        return;
    }
    MOCOSEL_BYTE* from = node->layout.from;
    MOCOSEL_BYTE* to = node->layout.to;
    for(; from != to; from += sizeof(struct MOCOSEL_VALUE)) {
        struct MOCOSEL_VALUE* value = (struct MOCOSEL_VALUE*)from;
        if(value->type == MOCOSEL_TYPE_LIST) {
            MOCOSEL_PURGE((struct MOCOSEL_LIST*)value->data);
        }
        if(value->length > 0) {
            MOCOSEL_FREE(value->data);
        }
    }
    if(node->layout.from) {
        MOCOSEL_FREE(node->layout.from);
    }
    MOCOSEL_PURGE(node->node);
    if(node->node) {
        MOCOSEL_FREE(node->node);
    }
}
