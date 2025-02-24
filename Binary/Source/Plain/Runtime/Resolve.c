/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 08/30/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>

PLAIN_WORD_DOUBLE resolve(void* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    if(type == PLAIN_TYPE_LIST) { /* List. */
        PLAIN_LIST* node = (PLAIN_LIST*)data;
        /* Make sure it's not empty. */
        if(node->keyword.from != node->keyword.to) {
            PLAIN_WORD_DOUBLE index = 0;
            PLAIN_WORD_DOUBLE length = PLAIN_ARITY(node);
            /* Arguments after substitution. */
            for(; index < length; index++) {
                PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, index);
                if(argument->type == PLAIN_TYPE_BOOLEAN) { /* Boolean. */
                    printf("Resolving boolean %d.\n", argument->data != NULL);
                } else if(argument->type == PLAIN_TYPE_NIL) { /* Nil. */
                    printf("Resolving nil.\n");
                } else if(argument->type == PLAIN_TYPE_INTEGER) { /* Integer. */
                    printf("Resolving integer %d.\n", *(const PLAIN_WORD_DOUBLE*)argument->data);
                } else if(argument->type == PLAIN_TYPE_REAL) { /* Real. */
                    printf("Resolving real %f.\n", *(const PLAIN_REAL*)argument->data);
                } else if(argument->type == PLAIN_TYPE_STRING) { /* String. */
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
    } else if(type == PLAIN_TYPE_KEYWORD) {
        printf("Resolving variable %s.\n", (const char*)data);
    }
    return 0;
}
