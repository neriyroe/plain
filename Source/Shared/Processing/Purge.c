/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/08/2013,
 * Revision 12/22/2013,
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
            MOCOSEL_RESIZE(value->data, 0, value->length);
        }
    }
    if(node->layout.from) {
        MOCOSEL_RESIZE(node->layout.from, 0, node->layout.to - node->layout.from);
    }
    MOCOSEL_PURGE(node->node);
    if(node->node) {
        MOCOSEL_RESIZE(node->node, 0, sizeof(struct MOCOSEL_LIST));
    }
}
