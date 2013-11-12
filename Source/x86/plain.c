/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include "plain.h"

MOCOSEL_WORD_DOUBLE PLAIN_TYPE(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_VALUE* __restrict value) {
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE number = MOCOSEL_MEASURE(node);
    for(; index < number; index++) {
        struct MOCOSEL_VALUE* value = MOCOSEL_ARGUMENT(node, index);
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
            struct MOCOSEL_VALUE value;
            MOCOSEL_WALK(node, registry, &value);
            if(node->keyword.from) {
                printf("PLAIN_TYPE MOCOSEL_LIST: %.*s, value = %s\n", node->keyword.to - node->keyword.from, (char*)node->keyword.from, (const char*)value.data);
            }
        } else if(value->type == MOCOSEL_TYPE_NIL) {
            printf("PLAIN_TYPE MOCOSEL_NIL.\n");
        }
    }
    return 0;
}

MOCOSEL_WORD_DOUBLE PLAIN_TRYING_A_NODE(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_VALUE* __restrict value) {
    if(node == NULL || registry == NULL || value == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    static const char str[] = "Hello World!";
    value->data = (MOCOSEL_BYTE*)str;
    value->length = strlen(str) + 1;
    value->type = MOCOSEL_TYPE_STRING;
    return 0;
}

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
        MOCOSEL_SYNTHESIZE("type", &object.registry.data, PLAIN_TYPE);
        MOCOSEL_SYNTHESIZE("trying-a-node", &object.registry.data, PLAIN_TRYING_A_NODE);
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
