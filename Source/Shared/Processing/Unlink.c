/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/08/2013,
 * Revision 01/11/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

void MOCOSEL_UNLINK(struct MOCOSEL_LIST* node) {
    if(node == NULL) {
        return;
    }
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(node);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* value = MOCOSEL_ARGUMENT(node, index);
        if(value->type == MOCOSEL_TYPE_LIST) {
            MOCOSEL_UNLINK((struct MOCOSEL_LIST*)value->data);
        }
        if(value->length > 0) {
            MOCOSEL_RESIZE(value->data, 0, value->length);
        }
    }
    if(node->layout.from) {
        MOCOSEL_RESIZE(node->layout.from, 0, node->layout.to - node->layout.from);
    }
    MOCOSEL_UNLINK(node->node);
    if(node->node) {
        MOCOSEL_RESIZE(node->node, 0, sizeof(struct MOCOSEL_LIST));
    }
}
