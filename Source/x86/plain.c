/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/11/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include "plain.h"

MOCOSEL_WORD_DOUBLE PLAIN_TYPE(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_VALUE* __restrict value) {
    MOCOSEL_ASSERT(node != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    if(node == NULL || registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_BYTE* from = node->layout.from;
    MOCOSEL_BYTE* to = node->layout.to;
    for(; from != to; from += sizeof(struct MOCOSEL_VALUE)) {
        struct MOCOSEL_VALUE* value = (struct MOCOSEL_VALUE*)from;
        if(value->type == MOCOSEL_TYPE_STRING || value->type == MOCOSEL_TYPE_KEYWORD) {
            printf("PLAIN_TYPE MOCOSEL_STRING: %s\n", (char*)value->data);
        } else if(value->type == MOCOSEL_TYPE_INTEGER) {
            printf("PLAIN_TYPE MOCOSEL_INTEGER: %d\n", *(int*)value->data);
        } else if(value->type == MOCOSEL_TYPE_REAL) {
            printf("PLAIN_TYPE MOCOSEL_REAL: %f\n", *(float*)value->data);
        } else if(value->type == MOCOSEL_TYPE_BOOLEAN) {
            printf("PLAIN_TYPE MOCOSEL_BOOLEAN: %d.\n", (int)value->data);
        } else if(value->type == MOCOSEL_TYPE_LIST) {
            struct MOCOSEL_LIST* node = (struct MOCOSEL_LIST*)value->data;
            if(node->keyword.from) {
                printf("PLAIN_TYPE MOCOSEL_LIST: %.*s\n", node->keyword.to - node->keyword.from, (char*)node->keyword.from);
            }
        } else if(value->type == MOCOSEL_TYPE_NIL) {
            printf("PLAIN_TYPE MOCOSEL_NIL.\n");
        }
    }
    return 0;
}

#define PLAIN_SUBROUTINE_TYPE 0x00

static const char* PLAIN_SUBROUTINE_TABLE[] = {
    "type"
};

int main(int count, const char* layout[]) {
    struct MOCOSEL_MANIFEST manifest;
    if(MOCOSEL_VERSION(&manifest) == 0) {
        printf("Warning: Plain might not function properly on this platform or operating system.\n");
    }
    struct MOCOSEL_OBJECT object;
    /* NWW: one per object. */
    struct MOCOSEL_SEGMENT segment = {NULL, NULL};
    if(count > 1) {
        PLAIN_FETCH(layout[1], &segment);
        if(segment.from == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(MOCOSEL_COMPILE, &manifest, &object, &segment);
        if(error != 0) {
            printf("Failed compiling %s: code %d.\n", layout[1], error);
        }
        /* Testing. */
        struct MOCOSEL_STATEMENT PLAIN_STATEMENT_TYPE;
        PLAIN_STATEMENT_TYPE.first.from = (MOCOSEL_BYTE*)PLAIN_SUBROUTINE_TABLE[PLAIN_SUBROUTINE_TYPE];
        PLAIN_STATEMENT_TYPE.first.to = PLAIN_STATEMENT_TYPE.first.from + strlen(PLAIN_SUBROUTINE_TABLE[PLAIN_SUBROUTINE_TYPE]);
        PLAIN_STATEMENT_TYPE.second = &PLAIN_TYPE;
        MOCOSEL_REGISTER(&object.registry.data, &PLAIN_STATEMENT_TYPE);
        /* Testing. */
        MOCOSEL_FREE(segment.from);
        if(error == 0) {
            error = MOCOSEL_RUN(MOCOSEL_EXECUTE, &manifest, &object, NULL);
            if(error != 0) {
                printf("Failed executing %s: code %d.\n", layout[1], error);
            }
        }
        MOCOSEL_FINALIZE(&object);
    } else {
        printf("Usage: plain <source>.\n");
    }
    return 0;
}
