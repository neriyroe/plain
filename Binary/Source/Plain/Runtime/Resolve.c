/*
 * Author   Nerijus Ramanauskas <nr@mocosel.com>,
 * Date     11/01/2014,
 * Revision 03/11/2015,
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE resolve(void* context, void* data, MOCOSEL_WORD_DOUBLE type, struct MOCOSEL_VALUE* value) {
    if(type == MOCOSEL_TYPE_LIST) {
        MOCOSEL_LIST* node = (MOCOSEL_LIST*)data;
        if(node->keyword.from != node->keyword.to) {
            MOCOSEL_WORD_DOUBLE index = 0;
            MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(node);
            for(; index < length; index++) {
                MOCOSEL_VALUE* argument = MOCOSEL_ARGUMENT(node, index);
                if(argument->type == MOCOSEL_TYPE_BOOLEAN) {
                    printf("Resolving boolean %d.\n", argument->data != NULL);
                } else if(argument->type == MOCOSEL_TYPE_NIL) {
                    printf("Resolving nil.\n");
                } else if(argument->type == MOCOSEL_TYPE_INTEGER) {
                    printf("Resolving integer %d.\n", *(const MOCOSEL_WORD_DOUBLE*)argument->data);
                } else if(argument->type == MOCOSEL_TYPE_REAL) {
                    printf("Resolving real %f.\n", *(const MOCOSEL_REAL*)argument->data);
                } else if(argument->type == MOCOSEL_TYPE_STRING) {
                    printf("Resolving string %s.\n", (const char*)argument->data);
                }
            }
            /* Shall not be substituted. */
            if(node->parent == NULL) {
                printf("Resolving statement %.*s.\n", (int)(node->keyword.to - node->keyword.from), (const char*)node->keyword.from);
            /* Shall store a substitution in <value>. Note that all memory occupied by <value> will be freed if <length> > 0. */
            } else {
                printf("Resolving expression %.*s.\n", (int)(node->keyword.to - node->keyword.from), (const char*)node->keyword.from);
            }
        }
    /* Shall store a substitution in <value>. Note that all memory occupied by <value> will be freed if <length> > 0. */
    } else if(type == MOCOSEL_TYPE_KEYWORD) {
        printf("Resolving variable %s.\n", (const char*)data);
    }
    return 0;
}
